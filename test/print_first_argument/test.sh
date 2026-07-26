#!/bin/bash

. ../common.sh

pre_test

./test fake>>test.txt
if [ "$(< test.txt)" == "./test" ]; then
	output "passed"
else
	output "executable failed"
fi
