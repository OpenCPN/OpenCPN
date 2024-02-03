FROM python:3.8.0-alpine3.10

RUN pip install websockets

COPY vendor/protocol.py /usr/local/lib/python3.8/site-packages/websockets/protocol.py

COPY *.py /usr/bin/
COPY entrypoint.sh /usr/bin/
RUN chmod +x /usr/bin/*.py

RUN mkdir /certs
COPY *.pem /certs/

WORKDIR /certs

EXPOSE 8765 8766
CMD ["sh", "/usr/bin/entrypoint.sh"]
