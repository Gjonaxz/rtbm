#!/usr/bin/python
# -*- coding: utf-8 -*-

import unittest
import sys
sys.path.append('/usr/sbin/')
from rtbm import *

# run:
# python -m unittest test_rtbm

class TestNotification(unittest.TestCase):
	def test_unencrypted_noauth(self):
		notifications = Notification("localhost", 25, "", "rtbm@example.org", "", "", "sysadmin1@example.org,sysadmin2@example.org")
		notifications.notifyAdministrators()

	def test_starttls_auth(self):
		notifications = Notification("localhost", 587, "starttls", "rtbm@example.org", "", "", "sysadmin1@example.org,sysadmin2@example.org")
		notifications.notifyAdministrators()

	def test_smtps_auth(self):
		notifications = Notification("localhost", 465, "smtps", "rtbm@example.org", "", "", "sysadmin1@example.org,sysadmin2@example.org")
		notifications.notifyAdministrators()


def suite():
	suite = unittest.TestSuite()
	suite.addTest(unittest.makeSuite(TestNotification))
	return suite

if __name__ == '__main__':
	#unittest.main()

	#suiteFew = unittest.TestSuite()
	#suiteFew.addTest(TestNotification("test_unencrypted_noauth"))
	#unittest.TextTestRunner(verbosity=2).run(suiteFew)

	unittest.TextTestRunner(verbosity=2).run(suite())
