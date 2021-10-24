package com.leidos.controller;


import com.leidos.loading.LoadingActions;
import com.leidos.unloading.UnloadingActions;
import com.baeldung.openapi.model.ContainerActionStatus;
import com.baeldung.openapi.model.ContainerRequest;
import com.baeldung.openapi.model.InspectionRequest;
import com.baeldung.openapi.model.InspectionStatus;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.leidos.inspection.InspectionActions;

import org.junit.jupiter.api.Test;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultMatchers;


@WebMvcTest( controllers = { InspectionController.class, LoadingController.class, UnloadingController.class})
public class ControllerTest {
    
    @Autowired
    private MockMvc mvc;

    @MockBean
    private LoadingActions mockLoadingActions;

    @MockBean
    private UnloadingActions mockUnloadingActions;

    @MockBean
    private InspectionActions mockInspectionActions;

    private ObjectMapper mapper = new ObjectMapper();

    /**
     * Test GET /loading {@link HttpStatus} is always OK.
     * @throws Exception
     */
    @Test
    public void testGetLoading() throws Exception{
       
        mvc.perform(MockMvcRequestBuilders.get("/loading/pending")).andExpect( MockMvcResultMatchers.status().isOk());
    }

    /**
     * Test GET /unloading {@link HttpStatus} is always OK.
     * @throws Exception
     */
    @Test
    public void testGetUnloading() throws Exception{
       
        mvc.perform(MockMvcRequestBuilders.get("/unloading/pending")).andExpect( MockMvcResultMatchers.status().isOk());
    }

    /**
     * Test GET /inspection {@link HttpStatus} is always OK.
     * @throws Exception
     */
    @Test
    public void testGetInspection() throws Exception{
       
        mvc.perform(MockMvcRequestBuilders.get("/inspection/pending")).andExpect( MockMvcResultMatchers.status().isOk());
    }

    // LoadingController Test -------------------------------------------------------------------------------------

    /**
     * Test POST /loading responses from {@link LoadingController}.
     * @throws Exception
     */
    @Test
    public void testPostLoading() throws Exception {

        // Test response for valid payload
        ContainerRequest request = new ContainerRequest();
        request.setVehicleId("vehicleId");
        request.setContainerId("containerId");

        mvc.perform(MockMvcRequestBuilders.post("/loading")
            .contentType(MediaType.APPLICATION_JSON)
            .content(mapper.writeValueAsString(request)))
                .andExpect(MockMvcResultMatchers.status().isCreated());
       
        
        // Test response for empty post
        mvc.perform(MockMvcRequestBuilders.post("/loading")
            .contentType(MediaType.APPLICATION_JSON)
            .content("")).
                andExpect(MockMvcResultMatchers.status().isBadRequest());

        // Test response for invalid post
        mvc.perform(MockMvcRequestBuilders.post("/loading")
            .contentType(MediaType.APPLICATION_JSON)
            .content("{ \"invalid\": \"json\"}"))
                .andExpect(MockMvcResultMatchers.status().isBadRequest());

    }

    /**
     * Test GET /loading/{vehicleId} responses from {@link LoadingController}.
     * @throws Exception
     */
    @Test
    public void testLoadingVehicleIdGet() throws Exception {

        ContainerActionStatus responseStatus = new ContainerActionStatus();
        responseStatus.setContainerId("containerId");
        responseStatus.setVehicleId("vehicleId");
        responseStatus.setStatus(ContainerActionStatus.StatusEnum.LOADING);
        Long curTim = System.currentTimeMillis();
        responseStatus.setRequested(curTim);


        Mockito.when(mockLoadingActions.getContainerActionStatus("vehicleId")).thenReturn(responseStatus);
       
         // Test response for get loading/{vehicleId} for existing request
         mvc.perform(MockMvcRequestBuilders.get("/loading/vehicleId"
            )).andExpect(MockMvcResultMatchers.status().isOk());
         mvc.perform(MockMvcRequestBuilders.get("/loading/vehicleId"
            )).andExpect(MockMvcResultMatchers.content().json(mapper.writeValueAsString(responseStatus))); 
        
         // Test response for get loading/{vehicleId} for non-existent request
         mvc.perform(MockMvcRequestBuilders.get("/loading/no-existent"
         )).andExpect(MockMvcResultMatchers.status().isBadRequest()); 
    }

    // UnloadingController Test -------------------------------------------------------------------------------------

    /**
     * Test POST /unloading responses from {@link UnloadingController}.
     * @throws Exception
     */
    @Test
    public void testPostUnloading() throws Exception {

        // Test response for valid payload
        ContainerRequest request = new ContainerRequest();
        request.setVehicleId("vehicleId");
        request.setContainerId("containerId");

        mvc.perform(MockMvcRequestBuilders.post("/unloading")
            .contentType(MediaType.APPLICATION_JSON)
            .content(mapper.writeValueAsString(request)))
                .andExpect(MockMvcResultMatchers.status().isCreated());
       
        
        // Test response for empty post
        mvc.perform(MockMvcRequestBuilders.post("/unloading")
            .contentType(MediaType.APPLICATION_JSON)
            .content("")).
                andExpect(MockMvcResultMatchers.status().isBadRequest());

        // Test response for invalid post
        mvc.perform(MockMvcRequestBuilders.post("/loading")
            .contentType(MediaType.APPLICATION_JSON)
            .content("{ \"invalid\": \"json\"}"))
                .andExpect(MockMvcResultMatchers.status().isBadRequest());

    }

    /**
     * Test GET /unloading/{vehicleId} responses from {@link UnloadingController}.
     * @throws Exception
     */
    @Test
    public void testUnloadingVehicleIdGet() throws Exception {

        ContainerActionStatus responseStatus = new ContainerActionStatus();
        responseStatus.setContainerId("containerId");
        responseStatus.setVehicleId("vehicleId");
        responseStatus.setStatus(ContainerActionStatus.StatusEnum.UNLOADING);
        Long curTim = System.currentTimeMillis();
        responseStatus.setRequested(curTim);


        Mockito.when(mockUnloadingActions.getContainerActionStatus("vehicleId")).thenReturn(responseStatus);
       
         // Test response for get loading/{vehicleId} for existing request
         mvc.perform(MockMvcRequestBuilders.get("/unloading/vehicleId"
            )).andExpect(MockMvcResultMatchers.status().isOk());
         mvc.perform(MockMvcRequestBuilders.get("/unloading/vehicleId"
            )).andExpect(MockMvcResultMatchers.content().json(mapper.writeValueAsString(responseStatus))); 
        
         // Test response for get loading/{vehicleId} for non-existent request
         mvc.perform(MockMvcRequestBuilders.get("/unloading/no-existent"
         )).andExpect(MockMvcResultMatchers.status().isBadRequest()); 
    }

    // UnloadingController Test -------------------------------------------------------------------------------------

    /**
     * Test POST /inspection responses from {@link InspectionController}.
     * @throws Exception
     */
    @Test
    public void testPostInspection() throws Exception {

        // Test response for valid payload
        InspectionRequest request = new InspectionRequest();
        request.setVehicleId("vehicleId");
        request.setContainerId("containerId");

        mvc.perform(MockMvcRequestBuilders.post("/inspection")
            .contentType(MediaType.APPLICATION_JSON)
            .content(mapper.writeValueAsString(request)))
                .andExpect(MockMvcResultMatchers.status().isCreated());
       
        
        // Test response for empty post
        mvc.perform(MockMvcRequestBuilders.post("/inspection")
            .contentType(MediaType.APPLICATION_JSON)
            .content("")).
                andExpect(MockMvcResultMatchers.status().isBadRequest());

        // Test response for invalid post
        mvc.perform(MockMvcRequestBuilders.post("/inspection")
            .contentType(MediaType.APPLICATION_JSON)
            .content("{ \"invalid\": \"json\"}"))
                .andExpect(MockMvcResultMatchers.status().isBadRequest());

    }

    /**
     * Test POST /inspection/holding responses from {@link InspectionController}.
     * @throws Exception
     */
    @Test
    public void testPostInspectionHolding() throws Exception {

        // Mock inspectionAction to have current action
        InspectionStatus responseStatus = new InspectionStatus();
        responseStatus.setContainerId("containerId");
        responseStatus.setVehicleId("vehicleId");
        responseStatus.setStatus(InspectionStatus.StatusEnum.PENDING);
        Long curTim = System.currentTimeMillis();
        responseStatus.setRequested(curTim);


        Mockito.when(mockInspectionActions.getCurrentInspection()).thenReturn(responseStatus);

        // Test response for valid payload
        InspectionRequest request = new InspectionRequest();
        request.setVehicleId("vehicleId");
        request.setContainerId("containerId");

        mvc.perform(MockMvcRequestBuilders.post("/inspection/holding")
            .contentType(MediaType.APPLICATION_JSON)
            .content(mapper.writeValueAsString(request)))
                .andExpect(MockMvcResultMatchers.status().isCreated());
        
        // Test response for non current action ( both incorrect vehicle and container id)
        request.setVehicleId("wrong");
        mvc.perform(MockMvcRequestBuilders.post("/inspection/holding")
            .contentType(MediaType.APPLICATION_JSON)
            .content(mapper.writeValueAsString(request)))
                .andExpect(MockMvcResultMatchers.status().isBadRequest());

        request.setVehicleId("vehicleId");
        request.setContainerId("wrong");
        mvc.perform(MockMvcRequestBuilders.post("/inspection/holding")
            .contentType(MediaType.APPLICATION_JSON)
            .content(mapper.writeValueAsString(request)))
                .andExpect(MockMvcResultMatchers.status().isBadRequest());
        
        // Test response for empty post
        mvc.perform(MockMvcRequestBuilders.post("/inspection/holding")
            .contentType(MediaType.APPLICATION_JSON)
            .content("")).
                andExpect(MockMvcResultMatchers.status().isBadRequest());

        // Test response for invalid post
        mvc.perform(MockMvcRequestBuilders.post("/inspection")
            .contentType(MediaType.APPLICATION_JSON)
            .content("{ \"invalid\": \"json\"}"))
                .andExpect(MockMvcResultMatchers.status().isBadRequest());

    }

    /**
     * Test GET /inspection/{vehicleId} responses from {@link InspectionController}.
     * @throws Exception
     */
    @Test
    public void testInspectionVehicleIdGet() throws Exception {

        InspectionStatus responseStatus = new InspectionStatus();
        responseStatus.setContainerId("containerId");
        responseStatus.setVehicleId("vehicleId");
        responseStatus.setStatus(InspectionStatus.StatusEnum.PENDING);
        Long curTim = System.currentTimeMillis();
        responseStatus.setRequested(curTim);


        Mockito.when(mockInspectionActions.getInspectionStatus("vehicleId")).thenReturn(responseStatus);
       
         // Test response for get loading/{vehicleId} for existing request
         mvc.perform(MockMvcRequestBuilders.get("/inspection/vehicleId"
            )).andExpect(MockMvcResultMatchers.status().isOk());
         mvc.perform(MockMvcRequestBuilders.get("/inspection/vehicleId"
            )).andExpect(MockMvcResultMatchers.content().json(mapper.writeValueAsString(responseStatus))); 
        
         // Test response for get loading/{vehicleId} for non-existent request
         mvc.perform(MockMvcRequestBuilders.get("/inspection/no-existent"
         )).andExpect(MockMvcResultMatchers.status().isBadRequest()); 
    }

    
    
}
