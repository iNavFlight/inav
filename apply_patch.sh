#!/bin/bash

echo "Applying custom configuration patch..."

# Применяем патч
if git apply custom_config.patch; then
    echo "Patch applied successfully!"
else
    echo "Patch failed! Trying patch command..."
    patch -p1 < custom_config.patch
fi

# Создаем build директорию
mkdir -p build
cd build