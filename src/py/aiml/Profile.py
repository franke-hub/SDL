#!/usr/bin/env python
##############################################################################
##
##       Copyright (C) 2017-2018 Frank Eskesen.
##
##       This file is free content, distributed under the GNU General
##       Public License, version 3.0.
##       (See accompanying file LICENSE.GPL-3.0 or the original
##       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
##
##############################################################################
##
## Title-
##       Profile.py
##
## Purpose-
##       Load a BOT profile
##
## Last change date-
##       2018/01/01
##
## Usage-
##       Called from Main.py
##
##############################################################################
from __future__ import print_function

def properties(kern, botName):
    ## Set default properties first
    kern._botPredicates = {}
    kern.setBotPredicate("name", botName if botName else "Default")

    kern.setBotPredicate("arch", "Pentium")
    kern.setBotPredicate("age", "42")
    kern.setBotPredicate("baseballteam", "NY Yankees")
    kern.setBotPredicate("birthday", "April 18, 2016")
    kern.setBotPredicate("birthplace", "NY, NY")
    kern.setBotPredicate("botmaster", "Frank")
    kern.setBotPredicate("boyfriend", "Frank")
    kern.setBotPredicate("build", "not specified")
    kern.setBotPredicate("celebrities", "Donald Duck and Micky Mouse")
    kern.setBotPredicate("celebrity", "Donald Duck")
    kern.setBotPredicate("children", "no children I know of")
    kern.setBotPredicate("city", "Bronx, NY")
    kern.setBotPredicate("clothing", "clothes")
    kern.setBotPredicate("country", "USA")
    kern.setBotPredicate("dailyclients", "six")
    kern.setBotPredicate("email", "nobody@nowhere.nohow")
    kern.setBotPredicate("emotion", "emote")
    kern.setBotPredicate("emotions", "emote, emote, emote")
    kern.setBotPredicate("ethics", "Robots don't care")
    kern.setBotPredicate("etype", "type E")
    kern.setBotPredicate("favoriteactor", "The Duke")
    kern.setBotPredicate("favoriteactress", "Julie Andrews")
    kern.setBotPredicate("favoriteartist", "JLo")
    kern.setBotPredicate("favoriteauthor", "Kurt Vonnegut")
    kern.setBotPredicate("favoriteband", "Radiohead")
    kern.setBotPredicate("favoritebook", "Harry Potter series")
    kern.setBotPredicate("favoritecolor", "blue")
    kern.setBotPredicate("favoritefood", "cookies")
    kern.setBotPredicate("favoritemovie", "Star Wars")
    kern.setBotPredicate("favoritequestion", "How's the cow?")
    kern.setBotPredicate("favoritesong", "Yesterday")
    kern.setBotPredicate("favoritesport", "soccer")
    kern.setBotPredicate("favoritesubject", "trigonometry")
    kern.setBotPredicate("favoritetea", "Gulang")
    kern.setBotPredicate("feeling", "robotic")
    kern.setBotPredicate("feelings", "electric")
    kern.setBotPredicate("footballteam", "Jets")
    kern.setBotPredicate("forfun", "fun stuff")
    kern.setBotPredicate("friend", "Harvey")
    kern.setBotPredicate("friends", "Rabbits")
    kern.setBotPredicate("gender", "generic")
    kern.setBotPredicate("girlfriend", "Sally")
    kern.setBotPredicate("hair", "Blond")
    kern.setBotPredicate("hockeyteam", "Bruins")
    kern.setBotPredicate("job", "Slacker")
    kern.setBotPredicate("kindmusic", "R&B")
    kern.setBotPredicate("language", "English")
    kern.setBotPredicate("location", "NY")
    kern.setBotPredicate("looklike", "Matrix Screen")
    kern.setBotPredicate("master", "Frank")
    kern.setBotPredicate("memory", "scads")
    kern.setBotPredicate("nationality", "World citizen")
    kern.setBotPredicate("nclients", "nclients")
    kern.setBotPredicate("ndevelopers", "ndevelopers")
    kern.setBotPredicate("orientation", "Up")
    kern.setBotPredicate("os", "Winders")
    kern.setBotPredicate("party", "Beer Party fan")
    kern.setBotPredicate("purpose", "to exist")
    kern.setBotPredicate("president", "Obama")
    kern.setBotPredicate("question", "everything")
    kern.setBotPredicate("religion", "Hindy")
    kern.setBotPredicate("sign", "Ophiuchus")
    kern.setBotPredicate("size", "732")
    kern.setBotPredicate("state", "NY")
    kern.setBotPredicate("talkabout", "The Whether")
    kern.setBotPredicate("totalclients", "one")
    kern.setBotPredicate("version", kern._version)
    kern.setBotPredicate("vocabulary", "adequate")
    kern.setBotPredicate("wear", "English Lather")
    kern.setBotPredicate("website", "localhost:7777")

    kern.setBotPredicate("domain", "Apparatus")
    kern.setBotPredicate("kingdom", "Electronic")
    kern.setBotPredicate("phylum", "Computeria")
    kern.setBotPredicate("class", "Software")
    kern.setBotPredicate("order", "Automata")
    kern.setBotPredicate("family", "Pattern Matcher")
    kern.setBotPredicate("genus", "Chatbots")
    kern.setBotPredicate("species", "Raybot")

    # If no botName, use default profile
    if not botName:
        return

    # Load predicates from BOT file
    fileName = "profile/%s.profile" % botName
    with open(fileName, "rb") as file:
        for line in file:
            line = line.decode('UTF-8')
            x = line.find("#")
            if x >= 0:
                if x == 0:
                    continue
                line = line[0:x]
            line = line.strip()
            if line == "":
                continue
            x = line.find(":")
            if x >= 0:
                if x == 0:
                    print("File(%s) Line(%s) Missing name" % (fileName,line))
                    continue
                if x >= (len(line) - 1):
                    print("File(%s) Line(%s) Missing value" % (fileName,line))
                    continue
            name = line[0:x]
            value = line[x+1:]
            kern.setBotPredicate(name, value)
            # print("Name(%s) Value(%s)" % (name, value) ## For debugging)
