#!/bin/bash

. ../common.sh

pre_test

./test arg THREE >>test.txt
if [ "$(< test.txt)" == "./testargTHREE" ]; then
	output "passed"
else
	output "executable failed"
fi
