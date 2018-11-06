# http-server
## How to launch
To launch this project, simply issue the following command:

```
./lisod <port number> <placeholder value> <log directory> <place holder> <www directory to use>
```

There are placeholder values because the autograde script needs them in order to launch the server and perform testing. These parameters aren't used in the program though. If the supplied command parameters are not all there, the program exits with error. 

## How I tested...

There are several ways in which I deployed to test my project:

1. For checkpoint1, I used the echo_client, checkpoint1 script, and telnet to make sure that the server is echoing back my inputs.

2. For GET and HEAD, I used a combination of test cases with CURL along with Chrome developer tool to check the responses from my server. Some of my test cases were:

* I issued the following command for testing:

	```curl -v -H "$(cat sample_request_realistic)" 127.0.0.1:9999```

* I changed the header in the sample_request_realistic file for testing the following cases:
	
	* malformed header 
		* making the header extremely long (past the 8192 byte limit)
		* making the screwing up the line endingA
		* both these test cases should return "Bad Request"
	* http version
		* I changed the http version to 1.0 to see if I can fail it
	* I tried use a unsupported method to see if "Not Implemented was returned"
	* I tried to access a non-existant resource to see if "Not Found" was returned

* I also tried getting different extension files to make sure the response returns the correct mime type
	
3. For POST, I used some variation of the following command to attach a body:

```curl -H "Content-Length: 20" -X POST -d '{"username":"xyz", "password":"xyz"}' 127.0.0.1:9999```

- I printed the request body to the log file to check if post was parsing the correct string:
	
	* I tried not supplying a content-length to make sure "Length Required" was returned 
	* some other contents I tried sending through the post body
		* make the body of the post quite big (e.g. a file)
		* md5 it to make sure the results were the same on client and server

4. I also used apachebench to test my program which was supplied in the autograde tar but not used in the python script. I issued the following command:

```./apachebench/ab -kc 100 -n 40000 http://127.0.0.1:9999/index.html > ../tmp/ab.log```

* I then grepped the log for the failed key word and check if the command error rate was greater than 40000

5. Finally, for submission I ran the autograde script multiple times to check if my server would consistently pass, this is done through shell scripting (e.g.)

```
	for i in {0..100}
	do
		make 2>&1 > test_results.txt
		if grep -q -i "error" test_results.txt ; then
			echo "Failed"
			break
		fi
	done
```


-
