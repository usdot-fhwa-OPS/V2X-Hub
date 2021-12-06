package com.leidos.unloading;

import com.baeldung.openapi.model.ActionStatusList;
import com.baeldung.openapi.model.ContainerActionStatus;
import com.baeldung.openapi.model.ContainerRequest;
import com.baeldung.openapi.model.ContainerActionStatus.StatusEnum;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

/**
 * {@link UnloadingActions} is a spring bean for managing unloading actions.
 * Only allows for a single unloading action to be in progress at a time and is
 * stored as the {@link UnloadingActions#currentAction}. Any unloading actions
 * requested while the current action is not yet completed will be added to
 * {@link UnloadingActions#pendingActions} list. This list is then used to
 * populate the current action after an action is completed. Each completed
 * action is stored in the {@link UnloadingActions#completedActions} list.
 * 
 * @author Paul Bourelly
 */
@Component
public class UnloadingActions {

    private static Logger logger = LoggerFactory.getLogger(UnloadingActions.class);
    // List of all pending unloading actions
    private ActionStatusList pendingActions = new ActionStatusList();

    // List of completed unloading actions
    private ActionStatusList completedActions = new ActionStatusList();

    // Current unloading Action
    private ContainerActionStatus currentAction;

    /**
     * Empty Constructor
     */
    public UnloadingActions() {
    }

    /**
     * Getter for pending unloading actions
     * 
     * @return {@link ActionStatusList} list of pending unloading actions
     */
    public ActionStatusList getPendingActions() {
        return pendingActions;
    }

    /**
     * Getter for completed Actions
     * 
     * @return {@link ActionStatusList} list of completed actions
     */
    public ActionStatusList getCompletedActions() {
        return completedActions;
    }

    /**
     * Getter for current Action
     * 
     * @return {@link ContainerActionStatus} current in progress unloading
     */
    public ContainerActionStatus getCurrentAction() {
        return currentAction;
    }

    /**
     * Create {@link ContainerActionStatus} for valid {@link ContainerRequest}s.
     * Valid requests require vehicleId and containerId. Valid request will be set
     * as {@link UnloadingActions#currentAction} if no current action is present or
     * added list of pending actions.
     * 
     * @param request {@link ContainerRequest} Request to load container on to a
     *                vehicle
     */
    public void requestUnloadingAction(ContainerRequest request) {
        if (request != null) {
            ContainerActionStatus requestedAction = new ContainerActionStatus();
            requestedAction.setContainerId(request.getContainerId());
            requestedAction.setVehicleId(request.getVehicleId());
            requestedAction.setActionId(request.getActionId());
            requestedAction.setStatus(ContainerActionStatus.StatusEnum.PENDING);
            requestedAction.setRequested(System.currentTimeMillis());
            // Add action to list of pending actions if an
            // action is already in progress. Otherwise set
            // as current action
            if (currentAction != null)
                pendingActions.addActionsItem(requestedAction);
            else
                currentAction = requestedAction;
        } else {
            logger.warn("Attempted to add null ContainerRequest!");
        }

    }

   /**
     * Searches all unloading actions ( pending, current and completed ) for a given
     * actionId and returns {@link ContainerActionStatus}. Returns null if non is found.
     * 
     * @param actionId unique string to identify action
     * @return {@link InspectionStatus} for given action. Null if no action is found 
     * or null action id is provided.
     */
    public ContainerActionStatus getContainerActionStatus(String actionId) {
        if (actionId != null) {
            // First search pending loading actions ( null check since actions array is null
            // before adding values )
            if (pendingActions.getActions() != null) {
                for (ContainerActionStatus containerAction : pendingActions.getActions()) {
                    if (actionId.equals(containerAction.getActionId())) {
                        return containerAction;
                    }
                }
            }
            // search current action
            if (currentAction != null && actionId.equals(currentAction.getActionId())) {
                return currentAction;
            }
            // Search completed loading actions ( null check since actions array is null
            // before adding values )
            if (completedActions.getActions() != null) {
                for (ContainerActionStatus containerAction : completedActions.getActions()) {
                    if (actionId.equals(containerAction.getActionId())) {
                        return containerAction;
                    }
                }
            }
            logger.warn(String.format("No unloading actions exist with action ID %s !", actionId));
            return null;
        }
        logger.warn("Null action id is not valid!");
        return null;
    }

    /**
     * Mark {@link UnloadingActions#currentAction} as in progress by setting the
     * {@link ContainerActionStatus#status} to {@link StatusEnum#UNLOADING}.
     */
    public void startCurrentAction() {
        if (currentAction != null) {
            currentAction.setStatus(StatusEnum.UNLOADING);
        } else
            logger.warn("There is no current action!");
    }

    /**
     * Mark {@link UnloadingActions#currentAction} as complete and move it to the
     * completedActions list. If there are pending actions, set the next pending
     * action to current action, else set current action to null.
     */
    public void completeCurrentAction() {
        if (currentAction != null) {
            currentAction.setStatus(StatusEnum.UNLOADED);
            currentAction.setCompleted(System.currentTimeMillis());
            completedActions.addActionsItem(currentAction);
            // Remove first item in list of pending actions and set it to current action
            if (pendingActions.getActions() != null && !pendingActions.getActions().isEmpty()) {
                currentAction = pendingActions.getActions().remove(0);
            } else {
                currentAction = null;
            }
        } else
            logger.warn("There is no current action!");

    }

     /**
     * Clear all unloading actions
     */
    public void clear() {
        if ( pendingActions.getActions() != null )
            pendingActions.getActions().clear();
        if ( completedActions.getActions() != null )
            completedActions.getActions().clear();
        currentAction = null;
    }
}
