from __future__ import print_function
import sys
import requests


domain = '192.168.1'


def ip_block():
    global domain

    for i in range(0, 256):
        yield domain + '.' + str(i)


def dot():
    print(".", end="")
    sys.stdout.flush()


def attempt(ip):
    url = 'http://' + ip + '/areyoukeybert'
    try:
        request = requests.get(url, timeout=0.2)
        return request.text
    except:
        return ''


def scan():
    ip = None
    for ip in ip_block():
        result = attempt(ip)
        if result == 'IAmKeybert':
            print("\n" + ip)
            break
        else:
            dot()


if __name__ == "__main__":
    scan()
    print("\n")
