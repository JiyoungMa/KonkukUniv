Performance Evaluation
======================
## 1. 이상적으로 나온 경우 (5번 시행 후, 가장 평균과 가까운 값을 가져옴)
![그림1](https://user-images.githubusercontent.com/50768959/87394832-47c06900-c5eb-11ea-9919-49530c1a0a46.png)
![그림2](https://user-images.githubusercontent.com/50768959/87394836-48f19600-c5eb-11ea-80eb-ca5201e7726f.png)
##### 프로세스 수가 늘어남에 따라 걸리는 real time과 user tiem이 줄어들고 있음
##### => parallel하게 진행되고 있다는 것을 알 수 있음
##
## 2. 예상과 다르게 나온 경우
![그림3](https://user-images.githubusercontent.com/50768959/87394838-498a2c80-c5eb-11ea-8613-724a5a0a02ee.png)
![그림4](https://user-images.githubusercontent.com/50768959/87394839-498a2c80-c5eb-11ea-824b-11691742e1d1.png)
##### 예상에 따르면 프로세스가 가장 많은 100개일 때, 가장 시간이 적게 들어야 함
##### 하지만, 시간이 가장 믾아 들었다
##### => 프로세스 수가 늘어남에 따라 증가하는 fork(), wait() 횟수에 따라, 이에 드는 시간(비용)이 증가했기 때문
