# Стадия сборки: полноценный Linux с компилятором.
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN make clean && make && make check-static

FROM scratch

COPY --from=build /src/minibox /minibox

ENTRYPOINT ["/minibox"]
