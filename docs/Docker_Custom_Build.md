## Build the custom V2X Hub image

1.  Download V2X Hub using the instructions available on [Confluence](https://usdot-carma.atlassian.net/wiki/spaces/V2XH/pages/1886158849/V2X-Hub+Docker+Deployment).
2.  Changes to V2X Hub can then be made in your locally downloaded V2X Hub repo.
3.  Save and then copy those changes to your V2X Hub Docker container. Navigate to V2X-Hub/src/ and run:
```
sudo docker cp . v2xhub:/home/V2X-Hub/src/
```
4.  Navigate to the V2X-Hub/ directory and build the changes:
```
sudo docker build -t usdotfhwaops/v2xhub:<custom-tag-name> .
```
5.  There are two ways run the new container:
```
sudo docker stop v2xhub
sudo docker run usdotfhwaops/v2xhub:<custom-tag-name>
```
Or the v2xhub image tag can be changed in the docker-compose.yml and the docker-compose up/down command can be used.