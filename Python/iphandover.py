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



webhookURL = 'DISCORD WEBHOOK URL'
hostname = socket.gethostname()
localIP = get_ip()#socket.gethostbyname(hostname)

data = {
    'username': 'Sphere Rover',
    'embeds': [{
        'title': 'My local ipv4 address is:',
        'description': hostname + ': ' + localIP,
    }]
}

result = requests.post(webhookURL, json = data)

try:
    result.raise_for_status()
except:
    pass