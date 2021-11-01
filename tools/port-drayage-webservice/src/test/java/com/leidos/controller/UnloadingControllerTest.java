package com.leidos.controller;

import com.baeldung.openapi.model.ContainerActionStatus;
import com.baeldung.openapi.model.ContainerRequest;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.leidos.unloading.UnloadingActions;

import org.junit.jupiter.api.Test;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultMatchers;

@WebMvcTest(controllers = { UnloadingController.class })
public class UnloadingControllerTest {
    
    @Autowired
    private MockMvc mvc;

    @MockBean
    private UnloadingActions mockUnloadingActions;

    private ObjectMapper mapper = new ObjectMapper();

    /**
     * Test GET /unloading {@link HttpStatus} is always OK.
     * 
     * @throws Exception
     */
    @Test
    public void testGetUnloading() throws Exception {
            mvc.perform(MockMvcRequestBuilders.get("/unloading/pending"))
                            .andExpect(MockMvcResultMatchers.status().isOk());
    }

    /**
     * Test POST /unloading responses from {@link UnloadingController}.
     * 
     * @throws Exception
     */
    @Test
    public void testPostUnloading() throws Exception {

            // Test response for valid payload
            ContainerRequest request = new ContainerRequest();
            request.setVehicleId("vehicleId");
            request.setContainerId("containerId");
            request.setActionId("actionId");

            mvc.perform(MockMvcRequestBuilders.post("/unloading").contentType(MediaType.APPLICATION_JSON)
                            .content(mapper.writeValueAsString(request)))
                            .andExpect(MockMvcResultMatchers.status().isCreated());

            // Test response for empty post
            mvc.perform(MockMvcRequestBuilders.post("/unloading").contentType(MediaType.APPLICATION_JSON)
                            .content("")).andExpect(MockMvcResultMatchers.status().isBadRequest());

            // Test response for invalid post
            mvc.perform(MockMvcRequestBuilders.post("/unloading").contentType(MediaType.APPLICATION_JSON)
                            .content("{ \"invalid\": \"json\"}"))
                            .andExpect(MockMvcResultMatchers.status().isBadRequest());

    }

    /**
     * Test GET /unloading/{vehicleId} responses from {@link UnloadingController}.
     * 
     * @throws Exception
     */
    @Test
    public void testUnloadingVehicleIdGet() throws Exception {

            ContainerActionStatus responseStatus = new ContainerActionStatus();
            responseStatus.setContainerId("containerId");
            responseStatus.setVehicleId("vehicleId");
            responseStatus.setStatus(ContainerActionStatus.StatusEnum.UNLOADING);
            responseStatus.setRequested(System.currentTimeMillis());

            Mockito.when(mockUnloadingActions.getContainerActionStatus("vehicleId")).thenReturn(responseStatus);

            // Test response for get loading/{vehicleId} for existing request
            mvc.perform(MockMvcRequestBuilders.get("/unloading/vehicleId"))
                            .andExpect(MockMvcResultMatchers.status().isOk());
            mvc.perform(MockMvcRequestBuilders.get("/unloading/vehicleId")).andExpect(
                            MockMvcResultMatchers.content().json(mapper.writeValueAsString(responseStatus)));

            // Test response for get loading/{vehicleId} for non-existent request
            mvc.perform(MockMvcRequestBuilders.get("/unloading/no-existent"))
                            .andExpect(MockMvcResultMatchers.status().isBadRequest());
    }

}
