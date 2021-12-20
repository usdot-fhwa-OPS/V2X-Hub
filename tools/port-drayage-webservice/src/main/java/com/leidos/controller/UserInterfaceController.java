package com.leidos.controller;


import com.leidos.area.Area;
import com.leidos.area.AreaBean;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

@Controller
public class UserInterfaceController {
    
    @Autowired
    public AreaBean areaBean;


    @GetMapping("/")
    public String main( Model model) {
        if ( areaBean.getArea() != null )
            return "index";
        else {
            return "landing";
        }
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

    /**
     * Keeping access to landing page under this URL for testing purposes.
     * @return
     */
    @GetMapping("/debug/landing")
    public String main() {
        return "landing";
        
    }

}
