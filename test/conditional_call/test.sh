#!/bin/bash

. ../common.sh

pre_test

./test foo
if [ $? == 1 ]; then
	output "passed"
else
	output "executable failed"
fi
