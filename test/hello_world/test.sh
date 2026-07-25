#!/bin/bash

. ../common.sh

pre_test

./test >>test.txt
if [ "$(< test.txt)" == "Hello World!" ]; then
	output "passed"
else
	output "executable failed"
fi
