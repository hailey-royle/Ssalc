#!/bin/bash

. ../common.sh

pre_test

./test
if [ $? == 19 ]; then
	output "passed"
else
	output "executable failed"
fi

