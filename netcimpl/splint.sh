#!/bin/bash

splint -mustfreefresh -D__unix__= http.c +posixlib > splint.temp.txt
