package com.leidos.controller;

import com.baeldung.openapi.api.UiactionApi;
import com.leidos.area.Area;
import com.leidos.area.AreaBean;
import com.leidos.inspection.InspectionActions;
import com.leidos.loading.LoadingActions;
import com.leidos.unloading.UnloadingActions;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.RestController;


@RestController
public class UIActionController implements UiactionApi {

    private static Logger logger = LoggerFactory.getLogger(UIActionController.class);

    /**
     * Injected {@link LoadingActions} Spring Bean
     */
    @Autowired
    private LoadingActions loadingActions;

    /**
     * Injected {@link UnloadingActions} Spring Bean
     */
    @Autowired
    private UnloadingActions unloadingActions;

    /**
     * Injected {@link InspectionActions} Spring Bean
     */
    @Autowired
    private InspectionActions inspectionActions;

    /**
     * 
     */
    @Autowired
    private AreaBean areaBean;
    
    /**
     * {@inheritDoc}
     */
    @Override
    public ResponseEntity<Void> uiactionResetPost() {
        inspectionActions.clear();
        unloadingActions.clear();
        loadingActions.clear();
        logger.warn("Web Service Actions were cleared!");
        return new ResponseEntity<Void>(HttpStatus.OK);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ResponseEntity<Void> uiactionAreaAreaPost(String area) {
        if ( area.equals(Area.STAGING_AREA.getName())) {
            areaBean.stagingArea();
            return new ResponseEntity<>(HttpStatus.CREATED);
        }
        else if ( area.equals(Area.PORT_AREA.getName())) {
            areaBean.portArea();
            return new ResponseEntity<>(HttpStatus.CREATED);
        }
        else {
            return new ResponseEntity<>(HttpStatus.BAD_REQUEST);
        }
    }

}
