package com.leidos.controller;

import com.baeldung.openapi.api.ResetApi;
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
public class ResetController implements ResetApi {

    private static Logger logger = LoggerFactory.getLogger(ResetController.class);

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
     * {@inheritDoc}
     */
    @Override
    public ResponseEntity<Void> resetPost() {
        inspectionActions.clear();
        unloadingActions.clear();
        loadingActions.clear();
        logger.warn("Web Service Actions were cleared!");
        return new ResponseEntity<Void>(HttpStatus.OK);
    }
}
