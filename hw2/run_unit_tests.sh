#!/bin/bash
set -e

echo "Запуск unit-тестов сервисов MAX Disk 360"
echo "================================"
echo ""

echo "Unit-тесты Identity Service"
docker-compose -f docker-compose-tests.yaml run --rm identity-service-tests
echo ""

echo "Unit-тесты File Management Service"
docker-compose -f docker-compose-tests.yaml run --rm file-management-service-tests
echo ""

echo "Все тесты завершены. Результаты:"
echo "   - identity-service/test-results/identity-tests.xml"
echo "   - file-management-service/test-results/file-tests.xml"