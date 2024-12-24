IMAGENAME= my:my

CONTAINERNAME= nginxtest

all: build run exec

build:
	docker build --tag $(IMAGENAME) .

run:
	docker run -v /home/aes-sarg/Desktop/Webserv/:/webcontainer --name $(CONTAINERNAME) -p 8089:80 -d $(IMAGENAME) bash

exec:
	docker exec -it $(CONTAINERNAME) bash

logs:
	docker logs $(CONTAINERNAME)

stopAll:
	docker stop $(docker ps | awk '{print $1}' | sed -n '2,$p') 

clean:
	docker

fclean: clean
	docker system prune -af