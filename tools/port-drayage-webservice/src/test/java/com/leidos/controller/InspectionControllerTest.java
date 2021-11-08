package com.leidos.controller;

import com.baeldung.openapi.model.InspectionRequest;
import com.baeldung.openapi.model.InspectionStatus;
import com.baeldung.openapi.model.InspectionStatus.StatusEnum;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.leidos.inspection.InspectionActions;

import org.junit.jupiter.api.Test;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultMatchers;

/**
 * Test class for Inspection {@link RestController} bean
 */
@WebMvcTest(controllers = { InspectionController.class })
public class InspectionControllerTest {

	@Autowired
	private MockMvc mvc;

	@MockBean
	private InspectionActions mockInspectionActions;

	private ObjectMapper mapper = new ObjectMapper();

	/**
	 * Test GET /inspection {@link HttpStatus} is always OK.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testGetInspection() throws Exception {

		mvc.perform(MockMvcRequestBuilders.get("/inspection/pending")).andExpect(MockMvcResultMatchers.status().isOk());
	}

	/**
	 * Test POST /inspection responses from {@link InspectionController}.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testPostInspection() throws Exception {

		// Test response for valid payload
		InspectionRequest request = new InspectionRequest();
		request.setVehicleId("vehicleId");
		request.setContainerId("containerId");
		request.setActionId("actionId");

		mvc.perform(MockMvcRequestBuilders.post("/inspection").contentType(MediaType.APPLICATION_JSON)
				.content(mapper.writeValueAsString(request))).andExpect(MockMvcResultMatchers.status().isCreated());

		// Test response for empty post
		mvc.perform(MockMvcRequestBuilders.post("/inspection").contentType(MediaType.APPLICATION_JSON).content(""))
				.andExpect(MockMvcResultMatchers.status().isBadRequest());

		// Test response for invalid post
		mvc.perform(MockMvcRequestBuilders.post("/inspection").contentType(MediaType.APPLICATION_JSON)
				.content("{ \"invalid\": \"json\"}")).andExpect(MockMvcResultMatchers.status().isBadRequest());

		// Mock duplicate request
		InspectionStatus responseStatus = new InspectionStatus();
		responseStatus.setContainerId("containerId");
		responseStatus.setVehicleId("vehicleId");
		responseStatus.setActionId("actionId");
		responseStatus.setStatus(InspectionStatus.StatusEnum.PROCEED_TO_HOLDING);
		responseStatus.setRequested(System.currentTimeMillis());

		// Mock already existing action for action ID
		Mockito.when(mockInspectionActions.getInspectionStatus(responseStatus.getActionId()))
				.thenReturn(responseStatus);

		mvc.perform(MockMvcRequestBuilders.post("/inspection").contentType(MediaType.APPLICATION_JSON)
				.content(mapper.writeValueAsString(request))).andExpect(MockMvcResultMatchers.status().isBadRequest());

	}

	/**
	 * Test POST /inspection/holding responses from {@link InspectionController}.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testPostInspectionHolding() throws Exception {

		// Mock inspectionAction to have current action
		InspectionStatus responseStatus = new InspectionStatus();
		responseStatus.setContainerId("containerId");
		responseStatus.setVehicleId("vehicleId");
		responseStatus.setActionId("actionId");
		responseStatus.setStatus(InspectionStatus.StatusEnum.PROCEED_TO_HOLDING);
		responseStatus.setRequested(System.currentTimeMillis());

		// Mock current inspection
		Mockito.when(mockInspectionActions.getCurrentInspection()).thenReturn(responseStatus);
		Mockito.when(mockInspectionActions.getInspectionStatus("actionId")).thenReturn(responseStatus);

		// Test response for valid payload
		InspectionRequest request = new InspectionRequest();
		request.setVehicleId("vehicleId");
		request.setContainerId("containerId");
		request.setActionId("actionId");

		// Correct
		mvc.perform(MockMvcRequestBuilders.post("/inspection/holding/" + request.getActionId())
				.contentType(MediaType.APPLICATION_JSON)).andExpect(MockMvcResultMatchers.status().isCreated());
		mvc.perform(MockMvcRequestBuilders.post("/inspection/holding/" + request.getActionId())
				.contentType(MediaType.APPLICATION_JSON)).andExpect(MockMvcResultMatchers.status().isCreated());

		// Test response for non current action ( both incorrect vehicle and container
		// id)
		request.setActionId("wrong");
		mvc.perform(MockMvcRequestBuilders.post("/inspection/holding/" + request.getActionId())
				.contentType(MediaType.APPLICATION_JSON)).andExpect(MockMvcResultMatchers.status().isBadRequest());

		// Test response for invalid post
		mvc.perform(MockMvcRequestBuilders.post("/inspection").contentType(MediaType.APPLICATION_JSON)
				.content("{ \"invalid\": \"json\"}")).andExpect(MockMvcResultMatchers.status().isBadRequest());

	}

	/**
	 * Test GET /inspection/{vehicleId} responses from {@link InspectionController}.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testInspectionVehicleIdGet() throws Exception {

		InspectionStatus responseStatus = new InspectionStatus();
		responseStatus.setContainerId("containerId");
		responseStatus.setVehicleId("vehicleId");
		responseStatus.setStatus(InspectionStatus.StatusEnum.PENDING);
		responseStatus.setRequested(System.currentTimeMillis());

		Mockito.when(mockInspectionActions.getInspectionStatus("vehicleId")).thenReturn(responseStatus);

		// Test response for get loading/{vehicleId} for existing request
		mvc.perform(MockMvcRequestBuilders.get("/inspection/vehicleId"))
				.andExpect(MockMvcResultMatchers.status().isOk());
		mvc.perform(MockMvcRequestBuilders.get("/inspection/vehicleId"))
				.andExpect(MockMvcResultMatchers.content().json(mapper.writeValueAsString(responseStatus)));

		// Test response for get loading/{vehicleId} for non-existent request
		mvc.perform(MockMvcRequestBuilders.get("/inspection/no-existent"))
				.andExpect(MockMvcResultMatchers.status().isBadRequest());
	}

	/**
	 * Test inspection/complete/{action_id} POST
	 */
	@Test
	public void testInspectionCompleteActionIdPost() throws Exception {
		// Create current action
		InspectionStatus responseStatus = new InspectionStatus();
		responseStatus.setContainerId("containerId");
		responseStatus.setVehicleId("vehicleId");
		responseStatus.setActionId("actionId");
		responseStatus.setStatus(StatusEnum.PENDING);
		responseStatus.setRequested(System.currentTimeMillis());

		// Mock return responseStatus as current action
		Mockito.when(mockInspectionActions.getCurrentInspection()).thenReturn(responseStatus);
		Mockito.when(mockInspectionActions.getInspectionStatus(responseStatus.getActionId()))
				.thenReturn(responseStatus);

		// Assert 201 response when current action ID is provided
		mvc.perform(MockMvcRequestBuilders.post("/inspection/complete/{actionId}", responseStatus.getActionId()))
				.andExpect(MockMvcResultMatchers.status().isCreated());

		// Assert 400 response when incorrect action ID is provided
		mvc.perform(MockMvcRequestBuilders.post("/inspection/complete/{actionId}", "wrong"))
				.andExpect(MockMvcResultMatchers.status().isBadRequest());

	}

	/**
	 * Test inspection/hold/{action_id} POST
	 */
	@Test
	public void testInspectionHoldActionIdPost() throws Exception {
		// Create current action
		InspectionStatus responseStatus = new InspectionStatus();
		responseStatus.setContainerId("containerId");
		responseStatus.setVehicleId("vehicleId");
		responseStatus.setActionId("actionId");
		responseStatus.setStatus(StatusEnum.PENDING);
		responseStatus.setRequested(System.currentTimeMillis());

		// Mock return responseStatus as current action
		Mockito.when(mockInspectionActions.getCurrentInspection()).thenReturn(responseStatus);
		Mockito.when(mockInspectionActions.getInspectionStatus(responseStatus.getActionId()))
				.thenReturn(responseStatus);

		// Assert 201 response when current action ID is provided
		mvc.perform(MockMvcRequestBuilders.post("/inspection/hold/{actionId}", responseStatus.getActionId()))
				.andExpect(MockMvcResultMatchers.status().isCreated());

		// Assert 400 response when incorrect action ID is provided
		mvc.perform(MockMvcRequestBuilders.post("/inspection/hold/{actionId}", "wrong"))
				.andExpect(MockMvcResultMatchers.status().isBadRequest());

	}
}
