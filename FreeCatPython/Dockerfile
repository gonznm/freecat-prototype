#FROM mtgupf/mir-toolbox
#RUN apt-get update && apt-get install -y python3-pip
#RUN pip3 install python-osc

FROM mtgupf/essentia:ubuntu18.04-python3

COPY apt-packages.txt /tmp/
RUN apt-get update \
    && xargs -a /tmp/apt-packages.txt apt-get install -y \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt /tmp/
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

#EXPOSE 9001/udp
#EXPOSE 9002/udp

#ENTRYPOINT [ "python3" ]