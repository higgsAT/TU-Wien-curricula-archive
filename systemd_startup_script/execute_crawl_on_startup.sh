#!/bin/bash

# additional information how to set this up (to run on startup) using systemd:
#https://www.linode.com/docs/guides/start-service-at-boot/

# wait 3 minutes before executing the command (to be sure the network connection is up and running before crawling starts)
sleep 3m

# run the (compiled) program (adjust this path):
/home/itsme/Desktop/git_repos/TU-Wien-curricula-archive/build/execute_crawl
