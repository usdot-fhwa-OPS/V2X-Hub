package com.leidos;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.EnableAutoConfiguration;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * Main class for SpringBoot application. {@link EnableAutoConfiguration} annotation allow spring to scan all sub-directories for defined beans.
 * 
 * @author Paul Bourelly 
 */
@EnableAutoConfiguration
@SpringBootApplication
public class Application {

	public static void main(String[] args) {
		SpringApplication.run(Application.class, args);
	}



}