# Multi Client Word Game

This game was created in CS367 (Computer Networks) @ Western Washington University

## Premise

Players are given a string of characters (Always containing one vowel) and must take turns submitting valid words using those characters. Players cannot use a word already submitted. The game continues until the specified number of turns is over.

## Specifications

The game useses socket connections to communication between the players and servers. The game features the trie data structure to store valid words. Players that timeout/quit send a socket connection to end the game. 
