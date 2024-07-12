all: build up
build:
	docker-compose build 
up:
	docker-compose up -d
ps:
	docker-compose  ps
down:
	docker-compose down
stop:
	docker-compose  stop

rmallctnr:
	docker container rm $$(docker container ls -aq)
rmallvol:
	docker volume rm $$(docker volume ls -q)

reset: rmallctnr rmallvol

 
re: down build up
web:
	docker exec -it web /bin/bash
db:
	docker exec -it postgresql /bin/bash
