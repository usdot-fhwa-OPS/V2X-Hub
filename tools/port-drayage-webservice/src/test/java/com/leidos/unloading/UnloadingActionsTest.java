package com.leidos.unloading;

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

public class UnloadingActionsTest {
    public UnloadingActions unloadingActions;

    /**
     * Init to run before each test
     */
    @BeforeEach
    public void init() {
        // Initialize Inspection Action Bean
        unloadingActions = new UnloadingActions();
    }

    /**
     * Test case to test {@link UnloadingActions#getContainerActionStatus(String)}
     * and {@link UnloadingActions#requestUnloadingAction(ContainerRequest)} for
     * different possible inputs.
     */
    @Test
    public void requestLoadingActionTest() {
        // Returns null when provided null vehicleId
        assertNull(unloadingActions.getContainerActionStatus(null));

        // requestLoadingAction does not throw exceptions for null parameters
        unloadingActions.requestUnloadingAction(null);

        // Populate unloading actions with unloading requests
        ContainerRequest req1 = new ContainerRequest();
        req1.setVehicleId("vehicleA");
        req1.setContainerId("containerA");
        req1.setActionId("actionA");

        // Returns null before unloading action is requested
        assertNull(unloadingActions.getContainerActionStatus(req1.getActionId()));

        // Returns action after unloading action is requested
        unloadingActions.requestUnloadingAction(req1);
        assertEquals(req1.getVehicleId(), unloadingActions.getCurrentAction().getVehicleId());
        assertEquals(req1.getContainerId(), unloadingActions.getCurrentAction().getContainerId());

        // Attempt to request new unloading action with already pending
        // unloading action
        ContainerRequest req2 = new ContainerRequest();
        req2.setVehicleId("vehicleC");
        req2.setContainerId("containerC");
        req2.setActionId("actionC");
        unloadingActions.requestUnloadingAction(req2);
        ContainerActionStatus status = unloadingActions.getCurrentAction();
        assertEquals(req1.getContainerId(), status.getContainerId());
    }

    /**
     * Test case to test
     * {@link LoadingAction#completeCurrentAction(ContainerActionStatus)} for
     * different possible parameters.
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
        assertNull(unloadingActions.getCurrentAction());
        unloadingActions.completeCurrentAction();

        // Run startCurrentAction with no current action
        assertNull(unloadingActions.getCurrentAction());
        unloadingActions.startCurrentAction();

        unloadingActions.requestUnloadingAction(req1);
        unloadingActions.requestUnloadingAction(req2);
        unloadingActions.requestUnloadingAction(req3);

        // Completed actions is empty before completing any loading action but
        // loading action is in unloadingActions list of in progress actions
        assertNull(unloadingActions.getCompletedActions().getActions());
        ContainerActionStatus req1Status = unloadingActions.getContainerActionStatus(req1.getActionId());
        ContainerActionStatus req2Status = unloadingActions.getContainerActionStatus(req2.getActionId());
        ContainerActionStatus req3Status = unloadingActions.getContainerActionStatus(req3.getActionId());

        // First requested action becomes current action and each action requested after
        // is added to
        // pending actions
        assertEquals(req1Status, unloadingActions.getCurrentAction());
        assertTrue(unloadingActions.getPendingActions().getActions().contains(req2Status));
        assertTrue(unloadingActions.getPendingActions().getActions().contains(req3Status));

        // Start current action sets status to UNLOADING
        assertEquals(StatusEnum.PENDING, unloadingActions.getCurrentAction().getStatus());
        unloadingActions.startCurrentAction();
        assertEquals(StatusEnum.UNLOADING, unloadingActions.getCurrentAction().getStatus());

        // Current action status is set to UNLOADED and added to completedActions list.
        // Current action is replaced with next action in pending actions
        // Complete req1
        unloadingActions.completeCurrentAction();

        assertTrue(unloadingActions.getCompletedActions().getActions().contains(req1Status));
        assertNotNull(req1Status.getCompleted());
        assertEquals(ContainerActionStatus.StatusEnum.UNLOADED, req1Status.getStatus());
        assertEquals(req2Status, unloadingActions.getCurrentAction());

        // Complete req2
        unloadingActions.completeCurrentAction();

        assertTrue(unloadingActions.getCompletedActions().getActions().contains(req2Status));
        assertNotNull(req2Status.getCompleted());
        assertEquals(ContainerActionStatus.StatusEnum.UNLOADED, req2Status.getStatus());
        assertEquals(req3Status, unloadingActions.getCurrentAction());
        assertTrue(unloadingActions.getPendingActions().getActions().isEmpty());

        // Complete req3
        unloadingActions.completeCurrentAction();

        assertNull(unloadingActions.getCurrentAction());
        assertEquals(3, unloadingActions.getCompletedActions().getActions().size());

        // Get ContainerActionStatus will return most recent action even when it is
        // completed
        // for a given vehicleID
        assertEquals(req3Status, unloadingActions.getContainerActionStatus(req3.getActionId()));
    }
}
