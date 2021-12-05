package com.leidos.controller;


import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

@Controller
public class UserInterfaceController {
    

    @GetMapping("/")
    public String main( Model model) {
        return "index";
    }

    @GetMapping("/loading")
    public String loading() {
        return "_loading";
        
    }
    @GetMapping("/unloading")
    public String unloading() {
        return "_unloading";
        
    }
    @GetMapping("/inspection")
    public String inspection() {
        return "_inspection";
        
    }
}
