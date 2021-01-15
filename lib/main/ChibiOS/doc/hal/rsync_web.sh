#!/bin/bash
rsync -avP -e ssh --delete --exclude=.* ./html/ gdisirio,chibios@web.sourceforge.net:/home/groups/c/ch/chibios/htdocs/docs3/hal
