package com.leidos.area;

import org.springframework.stereotype.Component;

@Component
public class AreaBean {
    
    private Area area;

    /**
     * Empty Constructor.
     */
    public AreaBean() {

    }

    public void stagingArea() {
        area = Area.STAGING_AREA;
    }

    public void portArea() {
        area = Area.PORT_AREA;
    }

    public Area getArea() {
        return area;
    }
}
