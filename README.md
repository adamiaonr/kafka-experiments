# kafka-experiments

Results of experiments in Grafana: https://adamiaonr.grafana.net/d/5CSjVXbnk/confluent-cloud?orgId=1&from=1684668600000&to=1684677600000

| # | start time | # messages | # producers | # consumers | linger.ms | batch.size | compression.type | errors |
|---|---|---|---|---|---|---|---|---|
| 1 | Sun 21 May 12:50:00 | 2500 | 10 | 1 | 5 | 16384 | none | - |
| 2 | Sun 21 May 13:00:00 | 2500 | 10 | 1 | 5 | 81920 | none | - |
| 3 | Sun 21 May 13:10:00 | 2500 | 10 | 1 | 10 | 81920 | none | - |
| 4 | Sun 21 May 13:20:00 | 2500 | 10 | 1 | 10 | 81920 | zstd | - |
| 5 | Sun 21 May 13:30:00 | 2500 | 10 | 1 | 50 | 163840 | zstd | - |
| 6 | Sun 21 May 13:50:00 | 2500 | 10 | 5 | 50 | 163840 | zstd | connect error on producer #5 |
| 7 | Sun 21 May 14:00:00 | 2500 | 100 | 1 | 50 | 163840 | zstd | - |
