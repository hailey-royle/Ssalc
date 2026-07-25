#!/bin/bash

. ../common.sh

pre_test

./test two 3 foura08d8ha0sdufh
if [ $? == 4 ]; then
	output "passed"
else
	output "executable failed"
fi
