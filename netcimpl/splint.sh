#!/bin/bash

splint -mustfreefresh +charintliteral -D__unix__= http.c +posixlib > splint.temp.txt
