import os
import json

config = {'accent': '#376594',
		  'appMenuTriangularTabs': True,
		  'applets': ['appmenu', 'windowlist',
					  'spacer', 'workspaces',
					  'volume', 'kblayout',
					  'datetime', 'splitter',
					  'usermenu'],
		  'autostart': [],
		  'background': '',
		  'dateFormat': 'MMM d',
		  'enableAnimation': True,
		  'favApps': [],
		  'firstDayOfWeek': 'Monday',
		  'fontFamily': 'Open Sans',
		  'fontSize': 10,
		  'iconTheme': '',
		  'menuIcon': '/usr/share/plainDE/menuIcon.png',
		  'menuText': 'Apps',
		  'panelHeight': 28,
		  'panelLocation': 'top',
		  'showDate': True,
		  'theme': 'light',
		  'timeFormat': 'h:mm AP'}


userName = os.getenv('USER')
dirPath = '/home/' + userName + '/.config/plainDE'

try:
	os.mkdir(dirPath)
except FileNotFoundError:
	os.mkdir('/home/' + userName + '/.config')
	os.mkdir(dirPath)

with open(dirPath + '/config.json', 'w') as configWriter:
	json.dump(config, configWriter)
