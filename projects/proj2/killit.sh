#! /bin/bash
for a in `ps -a | grep "proj2" | cut -d" " -f2`
	do
		kill $a
	done
