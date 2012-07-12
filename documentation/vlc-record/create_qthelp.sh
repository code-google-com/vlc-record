#!/bin/bash

LANGUAGES="en de ru pl"

for i in $LANGUAGES ; do
	qhelpgenerator help_$i.qhp -o help_$i.qch
	qcollectiongenerator help_$i.qhcp -o help_$i.qhc
done

