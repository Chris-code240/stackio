+-----------------------------+
|        Stackio              |
+-----------------------------+
| 1. Network Layer ..Done     |
| 2. HTTP Parser              |
| 3. Routing Engine           |
| 4. Execution Engine         |
| 5. Storage Engine           |
| 6. Persistence Layer        |
| 7. Auth + Middleware        |
| 8. Observability            |
+-----------------------------+


curl -X POST http://127.0.0.1:8080/token -d "{\"token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJleHAiOjE3NzE5MzM5MjEsImlhdCI6MTc3MTkzMDMyMSwiaXNzIjoiYXV0aDAiLCJ1c2VyX2lkIjoiMTIzNDUifQ.eUdPIIbz1FGy7CtMeldQ3YEVvZLfKh1r5Vt-GJ4-X6I\"}" -H "Content-Type: application/json"

curl -X POST http://127.0.0.1:8080/token -d "{\"token\":\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJJRCI6IjE5MTg1NjM2NDAiLCJleHAiOjE3NzIwNTIzNjQsImlhdCI6MTc3MjA0ODc2NCwiaXNzIjoiYXV0aDAifQ.y98ZB4kkYAgmWG7XIwh-2HbyYzoaiTLBSR4IOlryaGw\"}" -H "Content-Type: application/json"
