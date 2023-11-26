import socket
import requests
def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP



webhookURL = 'https://discord.com/api/webhooks/1178373329044914196/YNtaZku0tmbM9Iy0MSNHoqgoY6S2XJ4qasBynLkYw21TQm4VY3TR5P7fqTFKcMkhCopy'
hostname = socket.gethostname()
localIP = get_ip()#socket.gethostbyname(hostname)

data = {
    'username': 'Server IP Webhook',
    'embeds': [{
        'title': 'Server started!',
        'description': hostname + ': ' + localIP,
    }]
}

result = requests.post(webhookURL, json = data)

try:
    result.raise_for_status()
except:
    pass