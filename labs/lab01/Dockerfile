# Стадия сборки: полноценный Linux с компилятором.
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# Сборка идёт через make — вызов сборщика происходит из сборки образа,
# как требует ТЗ. check-static падает, если бинарник оказался динамическим.
RUN make clean && make && make check-static

# Финальный образ: пустая файловая система плюс один файл.
FROM scratch

COPY --from=build /src/minibox /minibox

ENTRYPOINT ["/minibox"]
