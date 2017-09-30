#/bin/bash

ps ajx | grep getbook | 
		grep -v grep  | 
			awk '{print $2}' | 
				xargs kill -9 && 
					rm -rf /var/run/getbook.pid && 
						make clean && 
							make
