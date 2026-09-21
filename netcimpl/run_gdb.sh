#!/bin/bash

export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1

gdb a.out