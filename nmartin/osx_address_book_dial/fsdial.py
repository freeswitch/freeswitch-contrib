#!/usr/bin/env python


#==============================================================
#Copyright (c) 2009 Nik Martin
#
#Permission is hereby granted, free of charge, to any person
#obtaining a copy of this software and associated documentation
#files (the "Software"), to deal in the Software without
#restriction, including without limitation the rights to use,
#copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the
#Software is furnished to do so, subject to the following
#conditions:
#
#The above copyright notice and this permission notice shall be
#included in all copies or substantial portions of the Software.
#
#The original author of this work, Nik Martin, must be credited  
#as the original author of this work.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#OTHER DEALINGS IN THE SOFTWARE.
#
#
#==============================================================

import string
import sys
from optparse import OptionParser
from ESL import *


def main(argv):
	# change these to match your configuration
	my_number = "1000" #@mydomain.com
	server_addr = "fs.ip.add.ress"
	server_port = "8021"
	server_auth = "ClueCon"
	
		
	try:
		
		parser = OptionParser()
		parser.add_option("-n", "--number", dest="number",
								help="number to dial")

		(options, args) = parser.parse_args()

		
		if not options.number:
				print "ERROR: destination number required"
				sys.exit(2)
				
		cmd = "api originate {ignore_early_media=true,origination_caller_id_number=" + options.number + ",effective_caller_id_number=" + my_number + "}user/" + my_number + " &transfer(" + options.number + " XML default)"
		print cmd
			
		con = ESLconnection(server_addr, server_port, server_auth)
	#are we connected?

		if con.connected:
			
			e=con.sendRecv(cmd)
			print e.getBody()

		else:

			print "Not Connected"
			sys.exit(2)

	except:
		print "Unexpected error:", sys.exc_info()[0]
		raise
	
if __name__ == "__main__":
    main(sys.argv[1:])