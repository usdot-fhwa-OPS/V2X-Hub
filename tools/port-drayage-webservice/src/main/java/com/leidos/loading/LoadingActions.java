package com.leidos.loading;

import com.baeldung.openapi.model.ActionStatusList;
import com.baeldung.openapi.model.ContainerActionStatus;
import com.baeldung.openapi.model.ContainerRequest;
import com.baeldung.openapi.model.ContainerActionStatus.StatusEnum;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

/**
 * {@link LoadingActions} is a spring bean for managing loading actions. Only
 * allows for a single loading action to be in progress at a time and is stored
 * as the {@link LoadingActions#currentAction}. Any loading actions requested
 * while the current action is not yet completed will be added to
 * {@link LoadingActions#pendingActions} list. This list is then used to
 * populate the current action after an action is completed. Each completed
 * action is stored in the {@link LoadingActions#completedActions} list.
 * 
 * @author Paul Bourelly
 */
@Component
public class LoadingActions {

    private static Logger logger = LoggerFactory.getLogger(LoadingActions.class);

    // List of all pending loading actions
    private ActionStatusList pendingActions = new ActionStatusList();

    // List of completed loading actions
    private ActionStatusList completedActions = new ActionStatusList();

    // Current Loading Action
    private ContainerActionStatus currentAction;

    /**
     * Empty Constructor.
     */
    public LoadingActions() {

    }

    /**
     * Getter for pending loading actions.
     * 
     * @return {@link ActionStatusList} list of pending loading actions
     */
    public ActionStatusList getPendingActions() {
        return pendingActions;
    }

    /**
     * Getter for completed Actions.
     * 
     * @return {@link ActionStatusList} list of completed actions
     */
    public ActionStatusList getCompletedActions() {
        return completedActions;
    }

    /**
     * Getter for current Action.
     * 
     * @return {@link ContainerActionStatus} current in progress loading
     */
    public ContainerActionStatus getCurrentAction() {
        return currentAction;
    }

    /**
     * Create {@link ContainerActionStatus} for valid {@link ContainerRequest}s.
     * Valid requests require vehicleId and containerId. Valid request will be set
     * as {@link LoadingActions#currentAction} if no current action is present or
     * added to list of pending actions.
     * 
     * @param request {@link ContainerRequest} Request to load container.
     */
    public void requestLoadingAction(ContainerRequest request) {
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
     * Searches all loading actions ( pending, current and completed ) for a given
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
            logger.warn(String.format("No loading actions exist with action ID %s !", actionId));
            return null;
        }
        logger.warn("Null action id is not valid!");
        return null;
    }

    /**
     * Mark {@link LoadingActions#currentAction} as in progress by setting the
     * {@link ContainerActionStatus#status} to {@link StatusEnum#LOADING}.
     */
    public void startCurrentAction() {
        if (currentAction != null) {
            logger.debug(String.format("Starting loading for action %s", currentAction.toString()));
            currentAction.setStatus(StatusEnum.LOADING);
        } else
            logger.warn("There is no current action!");
    }

    /**
     * Mark {@link LoadingActions#currentAction} as complete and move it to the
     * completedActions list. If there are pending actions, set the next pending
     * action to current action, else set current action to null.
     */
    public void completeCurrentAction() {
        if (currentAction != null) {
            currentAction.setStatus(ContainerActionStatus.StatusEnum.LOADED);
            currentAction.setCompleted(System.currentTimeMillis());
            logger.debug(String.format("Complete loading for action %s", currentAction.toString()));
            logger.debug(String.format("Complete loading for action %s", currentAction.toString()));
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
     * Clear all loading actions
     */
    public void clear() {
        if ( pendingActions.getActions() != null )
            pendingActions.getActions().clear();
        if ( completedActions.getActions() != null )
            completedActions.getActions().clear();
        currentAction = null;
    }

    /**
     * Method to return the number of waiting actions for UI tab notification
     */
    public int waitingActions() {
        if ( pendingActions.getActions() != null && !pendingActions.getActions().isEmpty() ) {
            // plus one for current action
            return pendingActions.getActions().size() + 1;
        }
        else if ( currentAction != null ) {
            return 1;
        }
        else {
            return 0;
        } 
    } 

}
