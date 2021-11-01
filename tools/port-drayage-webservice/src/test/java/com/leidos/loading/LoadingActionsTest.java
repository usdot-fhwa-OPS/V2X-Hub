package com.leidos.loading;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

import com.baeldung.openapi.model.ContainerActionStatus;
import com.baeldung.openapi.model.ContainerRequest;
import com.baeldung.openapi.model.ContainerActionStatus.StatusEnum;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class LoadingActionsTest {

    public LoadingActions loadingActions;

    /**
     * Init to run before each test
     */
    @BeforeEach
    public void init() {
        // Initialize Inspection Action Bean
        loadingActions = new LoadingActions();
    }

    /**
     * Test case to test {@link LoadingActions#getContainerActionStatus(String)} and
     * {@link LoadingActions#requestLoadingAction(ContainerRequest)} for different
     * possible inputs.
     */
    @Test
    public void requestLoadingActionTest() {
        // Returns null when provided null vehicleId
        assertNull(loadingActions.getContainerActionStatus(null));

        // requestLoadingAction does not throw exceptions for null parameters
        loadingActions.requestLoadingAction(null);

        // Populate Loading actions with Loading requests
        ContainerRequest req1 = new ContainerRequest();
        req1.setVehicleId("vehicleA");
        req1.setContainerId("containerA");
        req1.setActionId("actionA");

        // Returns null before loading action is requested
        assertNull(loadingActions.getContainerActionStatus(req1.getActionId()));
        assertNull(loadingActions.getCurrentAction());

        // Returns action after loading action is requested
        loadingActions.requestLoadingAction(req1);
        assertEquals(req1.getVehicleId(), loadingActions.getCurrentAction().getVehicleId());
        assertEquals(req1.getContainerId(), loadingActions.getCurrentAction().getContainerId());

        // Attempt to request new loading action for vehicle with already pending
        // loading action
        ContainerRequest req2 = new ContainerRequest();
        req2.setVehicleId("vehicleA");
        req2.setContainerId("containerC");
        req2.setActionId("actionC");
        loadingActions.requestLoadingAction(req2);
        ContainerActionStatus status = loadingActions.getCurrentAction();
        assertEquals(req1.getContainerId(), status.getContainerId());
    }

    /**
     * Test case to test
     * {@link LoadingAction#completeCurrentAction(com.baeldung.openapi.model.ContainerActionStatus)}
     * for different possible parameters.
     */
    @Test
    public void completeLoadingTest() {
        // Populate loading actions with loading requests
        ContainerRequest req1 = new ContainerRequest();
        req1.setVehicleId("vehicleA");
        req1.setContainerId("containerA");
        req1.setActionId("actionA");

        ContainerRequest req2 = new ContainerRequest();
        req2.setVehicleId("vehicleB");
        req2.setContainerId("containerB");
        req2.setActionId("actionB");

        ContainerRequest req3 = new ContainerRequest();
        req3.setVehicleId("vehicleC");
        req3.setContainerId("containerC");
        req3.setActionId("actionC");

        // Run completeCurrentAction with no current action
        assertNull(loadingActions.getCurrentAction());
        loadingActions.completeCurrentAction();

        // Run startCurrentAction with no current action
        assertNull(loadingActions.getCurrentAction());
        loadingActions.startCurrentAction();

        loadingActions.requestLoadingAction(req1);
        loadingActions.requestLoadingAction(req2);
        loadingActions.requestLoadingAction(req3);

        // Completed actions is empty before completing any loading action but
        // loading action is in loadingActions list of in progress actions
        assertNull(loadingActions.getCompletedActions().getActions());
        ContainerActionStatus req1Status = loadingActions.getContainerActionStatus(req1.getActionId());
        ContainerActionStatus req2Status = loadingActions.getContainerActionStatus(req2.getActionId());
        ContainerActionStatus req3Status = loadingActions.getContainerActionStatus(req3.getActionId());

        // First requested action becomes current action and each action requested after
        // is added to
        // pending actions
        assertEquals(req1Status, loadingActions.getCurrentAction());
        assertTrue(loadingActions.getPendingActions().getActions().contains(req2Status));
        assertTrue(loadingActions.getPendingActions().getActions().contains(req3Status));

        

        // Start current action sets status to LOADING
        assertEquals(StatusEnum.PENDING, loadingActions.getCurrentAction().getStatus());
        loadingActions.startCurrentAction();
        assertEquals(StatusEnum.LOADING, loadingActions.getCurrentAction().getStatus());

        // Current action status is set to LOADED and added to completedActions list.
        // Current action is replaced with next action in pending actions
        // Complete req1
        loadingActions.completeCurrentAction();

        assertTrue(loadingActions.getCompletedActions().getActions().contains(req1Status));
        assertNotNull(req1Status.getCompleted());
        assertEquals(ContainerActionStatus.StatusEnum.LOADED, req1Status.getStatus());
        assertEquals(req2Status, loadingActions.getCurrentAction());

        // Complete req2
        loadingActions.completeCurrentAction();

        assertTrue(loadingActions.getCompletedActions().getActions().contains(req2Status));
        assertNotNull(req2Status.getCompleted());
        assertEquals(ContainerActionStatus.StatusEnum.LOADED, req2Status.getStatus());
        assertEquals(req3Status, loadingActions.getCurrentAction());
        assertTrue(loadingActions.getPendingActions().getActions().isEmpty());

        // Complete req3
        loadingActions.completeCurrentAction();

        assertNull(loadingActions.getCurrentAction());
        assertEquals(3, loadingActions.getCompletedActions().getActions().size());

        // Get ContainerActionStatus will return most recent action even when it is
        // completed
        // for a given vehicleID
        assertEquals(req3Status, loadingActions.getContainerActionStatus(req3.getActionId()));

    }
}
