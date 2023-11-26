https://discord.com/api/webhooks/1178373329044914196/YNtaZku0tmbM9Iy0MSNHoqgoY6S2XJ4qasBynLkYw21TQm4VY3TR5P7fqTFKcMkhCopy

import socket
import requests
import urllib.request

webhookURL = '<webhook url>'
hostname = socket.gethostname()
external_ipv6 = urllib.request.urlopen('https://v6.ident.me').read().decode('utf8')
external_ipv4 = socket.gethostbyname(hostname)
#urllib.request.urlopen('https://v4.ident.me').read().decode('utf8')


data = {
    'username': 'https://discord.com/api/webhooks/1178373329044914196/YNtaZku0tmbM9Iy0MSNHoqgoY6S2XJ4qasBynLkYw21TQm4VY3TR5P7fqTFKcMkhCopy',
    'embeds': [{
        'title': 'Server started!',
        'description': hostname + '\n' + 'IPV6' + ': ' + '`' + external_ipv6 + '`'  + '\n\n' + 'IPV4 ' + ':'  + '`' + external_ipv4 + '`',
    }]
}

result = requests.post(webhookURL, json = data)

try:
    result.raise_for_status()
except:
    pass