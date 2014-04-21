# professorChecker

## 기능
* 초음파 센서를 이용하여 거리를 측정합니다.
 * 1초에 한번 씩 5번을 검사합니다.
 * 5개의 거리 데이터의 평균 값을 사용합니다.
* 일정 거리 이하로 측정 되면 서버로 데이터를 전송합니다.

## 구성
### H/W
* Arduino UNO
* Ultra sonic sensor
* WiFi shield
* LCD

### S/W
* node.js
* express