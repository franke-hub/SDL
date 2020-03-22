//
// greet.cs
//
// Authors:
//  Jonathan Pryor <jpryor@novell.com>
//
// Copyright (C) 2008 Novell (http://www.novell.com)
//
// Source: http://www.ndesk.org/Options
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Note: Copyright and licensing assumed since this test code came from the
//       examples within same distribution package as ../Options.cs
//
using System;
using System.Collections.Generic;
using NDesk.Options;

class Test {
	static int verbosity;

	public static void Main (string[] args)
	{
		bool show_help = false;
		List<string> names = new List<string> ();
		int repeat = 1;

		var p = new OptionSet () {
			{ "n|name=", "the {NAME} of someone to greet.",
			  v => names.Add (v) },
			{ "r|repeat=", 
				"the number of {TIMES} to repeat the greeting.\n" + 
					"this must be an integer.",
			  (int v) => repeat = v },
			{ "v", "increase debug message verbosity",
			  v => { if (v != null) ++verbosity; } },
			{ "h|help",  "show this message and exit", 
			  v => show_help = v != null },
		};

		List<string> extra;
		try {
			extra = p.Parse (args);
		}
		catch (OptionException e) {
			Console.Write ("greet: ");
			Console.WriteLine (e.Message);
			Console.WriteLine ("Try `greet --help' for more information.");
			return;
		}

		if (show_help) {
			ShowHelp (p);
			return;
		}

		string message;
		if (extra.Count > 0) {
			message = string.Join (" ", extra.ToArray ());
			Debug ("Using new message: {0}", message);
		}
		else {
			message = "Hello {0}!";
			Debug ("Using default message: {0}", message);
		}

		foreach (string name in names) {
			for (int i = 0; i < repeat; ++i)
				Console.WriteLine (message, name);
		}
	}

	static void ShowHelp (OptionSet p)
	{
		Console.WriteLine ("Usage: greet [OPTIONS]+ message");
		Console.WriteLine ("Greet a list of individuals with an optional message.");
		Console.WriteLine ("If no message is specified, a generic greeting is used.");
		Console.WriteLine ();
		Console.WriteLine ("Options:");
		p.WriteOptionDescriptions (Console.Out);
	}

	static void Debug (string format, params object[] args)
	{
		if (verbosity > 0) {
			Console.Write ("# ");
			Console.WriteLine (format, args);
		}
	}
}

