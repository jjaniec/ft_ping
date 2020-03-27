FROM	debian

RUN	apt-get update -yqq

RUN	apt-get install -y \
      make \
      gcc \
      git \
      gdb

WORKDIR /app

CMD	["/app/ft_ping"]
