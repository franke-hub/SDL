<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
//----------------------------------------------------------------------------
//
// Title-
//       Debugging.md
//
// Purpose-
//       Document difficult to debug problems.
//
// Last change date-
//       2025/05/01
//
-------------------------------------------------------------------------- -->

Copyright &copy; 2022-2025 Frank Eskesen.

This file is free content, distributed under cc by-sa version 4.0 with
attribution required.
(See accompanying file LICENSE.BY_SA-4.0 or the original contained within
https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)

----

This journal records problems that were particularly difficult to debug.

- [\[SSH Fails "kex_exchange_identification"\]](#kex_exchange_id) <br/>
- [\[Too Many Open Sockets\]](#too-many-sockets) <br/>

----

# <a id="kex_exchange_id">SSH fails reporting "kex_exchange_identification" error.</a>
## (This fix is pretty obscure.)

I have a LAN containing multiple physical and virtual machines.

After switching routers on my LAN, I ran into a strange issue:
**SSH from Windows to Fedora stopped working**, even though:

- All machines could ping each other
- Fedora → Windows SSH worked fine
- Windows → Windows SSH also worked
- Fedora could SSH everywhere

But when I tried to SSH from Windows (OpenSSH) to Fedora, I consistently got
this error:

kex_exchange_identification: read: Software caused connection abort banner
exchange: Connection to 192.168.50.xxx port 22: Software caused connection
abort

Meanwhile, Fedora logged this:

Connection reset by 192.168.50.xxx port [some port]

## The Fix

**I had to manually change Window's IP subnet mask from `255.255.255.0` to
`255.255.255.255`.**

Yes, really.

After that change, SSH to Fedora worked immediately.

## How I Found It

I have two virtual Ubuntu machines
- Ubuntu[0] could SSH everywhere
- Ubuntu[1] failed in the same way the Windows machines failed.

I compared the network settings on the two Ubuntu machines.
The only significant network difference was the subnet mask.

Setting the mask to `255.255.255.255` (which normally means no subnet )
_somehow_ fixed the handshake failure, even on the Windows machines.

## Why This Works

Maybe it was because my network configuration changed recently:

I did recently change my physical router.
The old one had a network address of '192.168.0.1', and
the new one uses the network address '192.168.50.1'.

I also have a virtual box virtual adapter on my windows machine
(which runs all my VirtualBox virtual machines)
has a network adapter of '192.168.56.1'.

My fedora machine also has a virtual box virtual adapter (virbr0) which has a
network adapter of 192.168.122.1.
It hasn't had any virtual machines configured for a long time.

### But the truth is, I don't know what causes this problem or why the fix works.

But if you're seeing this error *and everything else seems fine*, try this.

## How to fix it.

### On the router itself

[(Reference)](https://www.digitalcitizen.life/change-subnet-mask-windows-10/)
See: "How to change the Subnet Mask from the router's interface"

I didn't actually see this until writing this note and can't vouch for it,
but it looks like maybe I should've tried this first.

### Windows 10 or 11 with manual IP

1. Go to `Control Panel > Network and Internet > Network Connections`
2. Right-click your active adapter → Properties
3. Select **Internet Protocol Version 4 (TCP/IPv4)** → Properties
4. Set:
   - IP address: your manual IP
   - Subnet mask: `255.255.255.255`
   - Gateway and DNS as needed

Apply, save, and try SSH again.

### Windows using DHCP

[(Reference)](https://www.digitalcitizen.life/change-subnet-mask-windows-10/)
See: How to change the Subnet Mask in Windows using PowerShell

- Open an administrator Windows PowerShell prompt
- Use "Get-NetAdapter -physical" to list your network adapters.
Each adapter is associated with an ifIndex (Interface Index.)
- Use "Set-NetIPAddress -InterfaceIndex {number} -PrefixLength 32", which sets
your subnet mask to 255.255.255.255.

(Maybe there's more to it than this because the change didn't persist after a
reboot.)
Because the change didn't persist, I changed my DHCP Windows machine to static
IP and used the manual method instead.

### Ubuntu with manual IP

- Use the "Edit Connections" menu from network conections menu on the top of
the display screen.
- Select the network to change
- Change the netmask to 32 (indicating 32 bits, or 255,255,255,255)

### Fedora with manual IP

- nmcli connection show (Displays the device names)
- sudo ifconfig {device} netmask 255.255.255.255
- ifconfig ## Verify the change

### Reboot test

After making network changes on any machine, reboot to make sure they still
work. If there are problems, now's the time to fix them.

----

# <a id="too-many-sockets">Too Many Open Sockets</a>

At this writing, ~/src/cpp/lib/pub/Test/TestSock.cpp's HTTP stress test ran a
server under a separate thread, and a client that sent a request, read the
response, then closed the connection. On Linux, this stress test would run
for several seconds and then the client connection would fail with the error
"Cannot assign requested address." Furthermore, the server would not complete
normally. It had to be interrupted.

I approached the problem by going after the server problem first. Server's that
don't *always* shut down properly need fixing. I added a bunch of debugging
traceh() statements, trying to figure out exactly where the program was stuck.
I narrowed it down to ~/src/cpp/lib/pub/Socket.cpp, in Socket::accept. The
::accept wasn't completing. Furthermore, the logic in
TestSock.cpp to stop the StreamServer thread didn't work. Closing both the
listener socket and the client socket still left the ::accept in limbo.

More and more diagnostic tracing eventually found that there was some sort of
problem with port re-use. The accept would fail after about 30K operations,
and the last working connection port would be about the same as the first.

Looking for solutions I found:
- https://stackoverflow.com/questions/12565112/how-to-break-c-accept-function,
suggesting using select before accept to insure that the accept will be
accepted. This prevents a server hang, but not the client connection failures.
- https://stackoverflow.com/questions/3757289/when-is-tcp-option-so-linger-0-required,
describing when it's appropriate to use socket option SO_LINGER. There is some
discussion and debate among the responses about the question, leaning towards
avoid using it if possible.

However, a server can detect when a client closes a socket normally or there
is some sort of non-recoverable error. Using SO_LINGER in this instance allows
connection re-use in this normal case, so the client won't get a connection
failure the next time the same address/port pair is re-used. After a normal
close, the client doesn't get any indication when the connection is reset so
it should be safe to use then. After a transmission error, in-flight messages
are in limbo anyway so the SO_LINGER reset doesn't add any extra client
recovery complexity.

Note that only a linger with l_onoff= 1 and l_linger=0 prevents the socket from
going into TIME_WAIT state.

Use this sample code before closing your socket:

```
    struct linger option;
    option.l_onoff= 1;
    option.l_linger= 0;
    int rc= setsockopt(handle, SOL_SOCKET, SO_LINGER, &option, sizeof(option));
    if( rc != 0 ) { /* Replace this comment with your error recovery procedure */ }
```

Error recovery code is optional.
(Aside from writing an error message, there's really not a lot you can do.)

----
