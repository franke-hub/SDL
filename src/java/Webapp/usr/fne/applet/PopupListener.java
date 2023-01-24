//----------------------------------------------------------------------------
//
//       Copyright (C) 2008-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PopupListener.java
//
// Purpose-
//       Define PopupListener, extending MouseAdapter
//
// Last change date-
//       2020/01/15
//
//----------------------------------------------------------------------------
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import javax.swing.JPopupMenu;

//----------------------------------------------------------------------------
//
// Class-
//       PopupListener
//
// Purpose-
//       Base PopupListener, implementing required MouseAdapter methods.
//
//----------------------------------------------------------------------------
public class PopupListener extends MouseAdapter {
//----------------------------------------------------------------------------
// PopupListener.Attributes
//----------------------------------------------------------------------------
JPopupMenu             popup;       // The popup menu

//----------------------------------------------------------------------------
//
// Method-
//       PopupListener.PopupListener
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PopupListener(                   // Constructor
     JPopupMenu        menu)        // The popup menu
{
   popup= menu;
}

//----------------------------------------------------------------------------
//
// Method-
//       PopupListener.getPopup
//
// Purpose-
//       Accessor.
//
//----------------------------------------------------------------------------
public JPopupMenu                   // The associated JPopupMenu
   getPopup( )                      // Get associated JPopupMenu
{
   return popup;
}

//----------------------------------------------------------------------------
//
// Method-
//       PopupListener.mousePressed, mouseReleased, handle
//
// Purpose-
//       Handle mouse events.
//
//----------------------------------------------------------------------------
public void
   mousePressed(                    // MousePressed event
     MouseEvent        e)
{
   handle(e);
}

public void
   mouseReleased(                   // MouseReleased event
     MouseEvent        e)
{
   handle(e);
}

private void
   handle(                         // Event handler
     MouseEvent        e)
{
   if (e.isPopupTrigger())
     popup.show(e.getComponent(), 0, 0);
}
} // class PopupListener
