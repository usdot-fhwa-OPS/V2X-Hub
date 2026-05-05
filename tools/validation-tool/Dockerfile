FROM ubuntu:24.04

ENV DEBIAN_FRONTEND="noninteractive" TZ="America/New_York"
RUN apt-get update -y && apt-get install -y python3.12 python3.12-dev python3.12-venv python3.12-tk git unzip curl


RUN mkdir /validationTool
COPY ./src /validationTool
COPY ./requirements-py3.12.txt /validationTool/requirements.txt

WORKDIR /validationTool

# create venv with and install dependencies
RUN python3 -m venv .venv
ENV PATH="/validationTool/.venv/bin:$PATH"

RUN python -m pip install -r requirements.txt && \
    python -m pip install wheels/j2735_202409-0.1.0-py3-none-any.whl

ENTRYPOINT [ "python3" ]

CMD [ "validationTool.py" ]
