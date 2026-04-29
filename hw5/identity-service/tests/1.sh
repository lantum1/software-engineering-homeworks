#!/bin/bash

for i in {1..1050}; do
  status=$(curl -s -o /dev/null -w "%{http_code}" -X POST http://localhost:8080/v1/users/auth/register \
    -H "Content-Type: application/json" \
    -d "{\"phone\":\"+7999${i}000000\",\"email\":\"test${i}@example.com\",\"firstName\":\"Test\",\"lastName\":\"User\"}")
  if [ "$status" == "429" ]; then
    echo "✓ Блокировка на запросе #$i"
    break
  fi
  [ $((i % 100)) -eq 0 ] && echo "Отправлено $i запросов..."
done