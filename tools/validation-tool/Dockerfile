FROM ubuntu:18.04

ENV DEBIAN_FRONTEND="noninteractive" TZ="America/New_York"
RUN apt-get update -y && apt-get install -y python3-dev python3-pip git unzip python3-tk

RUN  python3 -m pip install --upgrade pip

RUN mkdir /validationTool
COPY . /validationTool

WORKDIR /validationTool
RUN git clone git@github.com:P1sec/pycrate.git
WORKDIR /validationTool/pycrate
RUN python3 setup.py install

WORKDIR /validationTool

RUN pip3 install -r requirements.txt

ENTRYPOINT [ "python3" ]

CMD [ "validationTool.py" ]
