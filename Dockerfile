FROM debian:buster AS base

RUN set -ex; \
    apt-get update;


FROM base AS builder

COPY . /breakpoint-proximity-chat

RUN set -ex; \
    apt-get install -y g++ cmake; \
    cd /breakpoint-proximity-chat; \
    rm -rf build || : ; \
    cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -Bbuild; \
    cmake --build ./build --config Release --target server;


FROM base AS runtime

WORKDIR /breakpoint-proximity-chat
COPY --from=builder /breakpoint-proximity-chat/build/server .
RUN mkdir database

CMD ["/breakpoint-proximity-chat/server", "9099", "database/positions.db"]
EXPOSE 9099
