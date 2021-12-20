package com.leidos.area;

public enum Area {
    STAGING_AREA("STAGING_AREA"),
    PORT_AREA("PORT_AREA");
    private String name;

    Area(String name){
        this.name =name;
    }
    
    public String getName(){
        return name;
    }

}
