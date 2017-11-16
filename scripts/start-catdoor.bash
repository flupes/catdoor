#!/bin/bash

# sleep to let ntp some time to sync
sleep 20

session_name=catdoor

catdir=/home/debian/devel/catdoor
today=`date +%Y-%m-%d_%H%M%S`
catlog=${catdir}/logs/${today}.log

byobu new-session -d -s ${session_name} -c ${catdir}

byobu new-window -t ${session_name}:1 -n pusher "/usr/bin/python -u ${catdir}/python/catdoor_pushbullet.py | tee ${catlog}"
#tmux new-window -t ${session_name}:1 -n pusher -c ${catdir}
#tmux send-keys -t ${session_name}:1 "/usr/bin/python ${catdir}/python/catdoor_pushbullet.py" enter

byobu new-window -d -t ${session_name}:2 -n top htop


