#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#
#!/bin/bash

# There a different options to use containers on target device:
# 1. push to your registry and pull on target
# 2. use import / export feature of e.g. Docker
# 3. Built on target (may be very slow)

IMAGE_BUILD=vsomeip-build:1.0
IMAGE_RELEASE=vsomeip-release:1.0
APP_NAME=generic

cd vsomeip-base
docker build -f Dockerfile.build -t $IMAGE_BUILD .
docker build  --platform linux/arm64/v8 --build-arg IMAGE_BUILD=$IMAGE_BUILD -f Dockerfile.release -t $IMAGE_RELEASE .

cd ../vsomeip-daemon
docker build --build-arg IMAGE_RELEASE=$IMAGE_RELEASE -f Dockerfile.release -t vsomeip-daemon-release:1.0 .

cd ../vsomeip-application
docker build --build-arg IMAGE_BUILD=$IMAGE_BUILD --build-arg IMAGE_RELEASE=$IMAGE_RELEASE --build-arg APP_NAME=$APP_NAME -f Dockerfile.release -t vsomeip-generic:1.0 .
