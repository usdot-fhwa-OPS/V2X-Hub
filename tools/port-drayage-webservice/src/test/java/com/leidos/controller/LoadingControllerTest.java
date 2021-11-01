package com.leidos.controller;

import com.baeldung.openapi.model.ContainerActionStatus;
import com.baeldung.openapi.model.ContainerRequest;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.leidos.loading.LoadingActions;

import org.junit.jupiter.api.Test;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultMatchers;

@WebMvcTest(controllers = { LoadingController.class })
public class LoadingControllerTest {

    @Autowired
    private MockMvc mvc;

    @MockBean
    private LoadingActions mockLoadingActions;

    private ObjectMapper mapper = new ObjectMapper();

    /**
     * Test GET /loading {@link HttpStatus} is always OK.
     * 
     * @throws Exception
     */
    @Test
    public void testGetLoading() throws Exception {

        mvc.perform(MockMvcRequestBuilders.get("/loading/pending"))
                            .andExpect(MockMvcResultMatchers.status().isOk());
    }

    /**
     * Test POST /loading responses from {@link LoadingController}.
     * 
     * @throws Exception
     */
    @Test
    public void testPostLoading() throws Exception {

        // Test response for valid payload
        ContainerRequest request = new ContainerRequest();
        request.setVehicleId("vehicleId");
        request.setContainerId("containerId");
        request.setActionId("actionId");

        mvc.perform(MockMvcRequestBuilders.post("/loading").contentType(MediaType.APPLICATION_JSON)
                        .content(mapper.writeValueAsString(request)))
                        .andExpect(MockMvcResultMatchers.status().isCreated());

        // Test response for empty post
        mvc.perform(MockMvcRequestBuilders.post("/loading").contentType(MediaType.APPLICATION_JSON).content(""))
                        .andExpect(MockMvcResultMatchers.status().isBadRequest());

        // Test response for invalid post
        mvc.perform(MockMvcRequestBuilders.post("/loading").contentType(MediaType.APPLICATION_JSON)
                        .content("{ \"invalid\": \"json\"}"))
                        .andExpect(MockMvcResultMatchers.status().isBadRequest());

    }

    /**
     * Test GET /loading/{vehicleId} responses from {@link LoadingController}.
     * 
     * @throws Exception
     */
    @Test
    public void testLoadingVehicleIdGet() throws Exception {

        ContainerActionStatus responseStatus = new ContainerActionStatus();
        responseStatus.setContainerId("containerId");
        responseStatus.setVehicleId("vehicleId");
        responseStatus.setActionId("actionId");
        responseStatus.setStatus(ContainerActionStatus.StatusEnum.LOADING);
        responseStatus.setRequested(System.currentTimeMillis());

        Mockito.when(mockLoadingActions.getContainerActionStatus("vehicleId")).thenReturn(responseStatus);

        // Test response for get loading/{vehicleId} for existing request
        mvc.perform(MockMvcRequestBuilders.get("/loading/vehicleId"))
                        .andExpect(MockMvcResultMatchers.status().isOk());
        mvc.perform(MockMvcRequestBuilders.get("/loading/vehicleId")).andExpect(
                        MockMvcResultMatchers.content().json(mapper.writeValueAsString(responseStatus)));

        // Test response for get loading/{vehicleId} for non-existent request
        mvc.perform(MockMvcRequestBuilders.get("/loading/no-existent"))
                        .andExpect(MockMvcResultMatchers.status().isBadRequest());
    }

    /**
     * Test loading/complete/{action_id} POST 
     */
    @Test
    public void testLoadingCompleteActionIdPost() throws Exception {
        // Create current action
        ContainerActionStatus responseStatus = new ContainerActionStatus();
        responseStatus.setContainerId("containerId");
        responseStatus.setVehicleId("vehicleId");
        responseStatus.setActionId("actionId");
        responseStatus.setStatus(ContainerActionStatus.StatusEnum.PENDING);
        responseStatus.setRequested(System.currentTimeMillis());

        // Mock return responseStatus as current action
        Mockito.when(mockLoadingActions.getCurrentAction()).thenReturn(responseStatus);

        // Assert 201 response when current action ID is provided
        mvc.perform(MockMvcRequestBuilders.post("/loading/start/{actionId}", responseStatus.getActionId() ) ).andExpect(MockMvcResultMatchers.status().isCreated());

        // Assert 400 response when incorrect action ID is provided
        mvc.perform(MockMvcRequestBuilders.post("/loading/start/{actionId}", "wrong" ) ).andExpect(MockMvcResultMatchers.status().isBadRequest());


    }


}
