#!/bin/bash

splint -D__unix__= http.c +posixlib > splint.txt
