#!/bin/bash

. ../common.sh

pre_test

./test
if [ $? == 173 ]; then
	output "passed"
else
	output "executable failed"
fi
