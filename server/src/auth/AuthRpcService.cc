/**
 * @file AuthRpcService.cc
 * @brief Implementation of authentication RPC service
 * @details This file contains the implementation of AuthRpcService class methods,
 *          including user registration, authentication, password management,
 *          account deletion, and exception handling for gRPC requests.
 */

#include "AuthRpcService.hpp"

#include <string_view>
#include <unordered_map>
#include <fmt/format.h>

namespace server_app::auth
{
    /// @brief Map exception types to error codes using table-driven approach
    const std::unordered_map<std::string_view, int> AuthRpcService::error_map_ = {
        {"already exists", 409}, // Conflict
        {"not found", 404}, // Not found
        {"locked", 423}, // Locked
        {"Invalid password", 401} // Unauthorized
    };

    /// @brief Helper function to validate request parameters
    template <typename RequestType, typename ValidatorFunc>
    [[nodiscard]] static std::optional<::grpc::Status> ValidateRequest(const RequestType* request, ValidatorFunc&& validator, const std::string& error_msg, ::rpc::AuthResponse* response) noexcept
    {
        if (!request || !validator(request))
        {
            response->set_success(false);
            response->set_message(error_msg);
            response->set_error_code(400);
            return ::grpc::Status{::grpc::StatusCode::INVALID_ARGUMENT, "Invalid request parameters"};
        }
        return std::nullopt; // No error, continue with normal processing
    }

    AuthRpcService::AuthRpcService(const std::string& db_path) noexcept : authenticator_(db_path)
    {
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::RegisterUser(::grpc::ServerContext* /*context*/, const ::rpc::RegisterUserRequest* const request, ::rpc::AuthResponse* const response)
    {
        // Validate request parameters using table-driven validation
        const auto validation_status = ValidateRequest(request, [](const ::rpc::RegisterUserRequest* req)
        {
            return !req->username().empty() && !req->password().empty();
        }, "Invalid request: username and password are required", response);

        if (validation_status)
        {
            return *validation_status;
        }

        try
        {
            const auto& username = request->username();
            const auto& password = request->password();
            const bool success = authenticator_.register_user(username, password);
            response->set_success(success);
            response->set_message(success ? "User registered successfully" : "Registration failed");
            return ::grpc::Status::OK;
        }
        catch (const common::exception::AuthenticationException& e)
        {
            return HandleAuthException(e, response);
        }
        catch (const std::exception& e)
        {
            response->set_success(false);
            response->set_message(fmt::format("System error: {}", e.what()));
            response->set_error_code(500);
            return {::grpc::StatusCode::INTERNAL, e.what()};
        }
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::AuthenticateUser(::grpc::ServerContext* /*context*/, const ::rpc::AuthenticateUserRequest* const request, ::rpc::AuthResponse* const response)
    {
        // Validate request parameters using table-driven validation
        const auto validation_status = ValidateRequest(request, [](const ::rpc::AuthenticateUserRequest* req)
        {
            return !req->username().empty() && !req->password().empty();
        }, "Invalid request: username and password are required", response);

        if (validation_status)
        {
            return *validation_status;
        }

        try
        {
            const auto& username = request->username();
            const auto& password = request->password();
            const auto auth_result = authenticator_.authenticate(username, password);

            response->set_success(auth_result.is_success());
            response->set_message(auth_result.error_message.empty() ? "Authentication successful" : auth_result.error_message);
            return ::grpc::Status::OK;
        }
        catch (const common::exception::AuthenticationException& e)
        {
            return HandleAuthException(e, response);
        }
        catch (const std::exception& e)
        {
            response->set_success(false);
            response->set_message(fmt::format("System error: {}", e.what()));
            response->set_error_code(500);
            return {::grpc::StatusCode::INTERNAL, e.what()};
        }
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::ChangePassword(::grpc::ServerContext* /*context*/, const ::rpc::ChangePasswordRequest* const request, ::rpc::AuthResponse* const response)
    {
        // Validate request parameters using table-driven validation
        const auto validation_status = ValidateRequest(request, [](const ::rpc::ChangePasswordRequest* req)
        {
            return !req->username().empty() && !req->current_password().empty() && !req->new_password().empty();
        }, "Invalid request: username, current password, and new password are required", response);

        if (validation_status)
        {
            return *validation_status;
        }

        try
        {
            const auto& username = request->username();
            const auto& current_password = request->current_password();
            const auto& new_password = request->new_password();

            const bool success = authenticator_.change_password(username, current_password, new_password);
            response->set_success(success);
            response->set_message(success ? "Password changed successfully" : "Password change failed");
            return ::grpc::Status::OK;
        }
        catch (const common::exception::AuthenticationException& e)
        {
            return HandleAuthException(e, response);
        }
        catch (const std::exception& e)
        {
            response->set_success(false);
            response->set_message(fmt::format("System error: {}", e.what()));
            response->set_error_code(500);
            return {::grpc::StatusCode::INTERNAL, e.what()};
        }
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::ResetPassword(::grpc::ServerContext* /*context*/, const ::rpc::ResetPasswordRequest* const request, ::rpc::AuthResponse* const response)
    {
        // Validate request parameters using table-driven validation
        const auto validation_status = ValidateRequest(request, [](const ::rpc::ResetPasswordRequest* req)
        {
            return !req->username().empty() && !req->new_password().empty();
        }, "Invalid request: username and new password are required", response);

        if (validation_status)
        {
            return *validation_status;
        }

        try
        {
            const auto& username = request->username();
            const auto& new_password = request->new_password();

            const bool success = authenticator_.reset_password(username, new_password);
            response->set_success(success);
            response->set_message(success ? "Password reset successfully" : "Password reset failed");
            return ::grpc::Status::OK;
        }
        catch (const common::exception::AuthenticationException& e)
        {
            return HandleAuthException(e, response);
        }
        catch (const std::exception& e)
        {
            response->set_success(false);
            response->set_message(fmt::format("System error: {}", e.what()));
            response->set_error_code(500);
            return {::grpc::StatusCode::INTERNAL, e.what()};
        }
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::DeleteUser(::grpc::ServerContext* /*context*/, const ::rpc::DeleteUserRequest* const request, ::rpc::AuthResponse* const response)
    {
        // Validate request parameters using table-driven validation
        const auto validation_status = ValidateRequest(request, [](const ::rpc::DeleteUserRequest* req)
        {
            return !req->username().empty();
        }, "Invalid request: username is required", response);

        if (validation_status)
        {
            return *validation_status;
        }

        try
        {
            const auto& username = request->username();
            const bool success = authenticator_.delete_user(username);
            response->set_success(success);
            response->set_message(success ? "User deleted successfully" : "User deletion failed");
            return ::grpc::Status::OK;
        }
        catch (const std::exception& e)
        {
            response->set_success(false);
            response->set_message(fmt::format("System error: {}", e.what()));
            response->set_error_code(500);
            return {::grpc::StatusCode::INTERNAL, e.what()};
        }
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::UserExists(::grpc::ServerContext* /*context*/, const ::rpc::UserExistsRequest* const request, ::rpc::AuthResponse* const response)
    {
        // Validate request parameters using table-driven validation
        const auto validation_status = ValidateRequest(request, [](const ::rpc::UserExistsRequest* req)
        {
            return !req->username().empty();
        }, "Invalid request: username is required", response);

        if (validation_status)
        {
            return *validation_status;
        }

        try
        {
            const auto& username = request->username();
            const bool exists = authenticator_.user_exists(username);
            response->set_success(exists);
            response->set_message(exists ? "User exists" : "User not found");
            return ::grpc::Status::OK;
        }
        catch (const std::exception& e)
        {
            response->set_success(false);
            response->set_message(fmt::format("System error: {}", e.what()));
            response->set_error_code(500);
            return {::grpc::StatusCode::INTERNAL, e.what()};
        }
    }

    [[nodiscard]] ::grpc::Status AuthRpcService::HandleAuthException(const common::exception::AuthenticationException& e, ::rpc::AuthResponse* const response) noexcept
    {
        response->set_success(false);
        response->set_message(e.what());

        const std::string error_msg = e.what();
        response->set_error_code(400); // Default error code (Bad request)

        // Use range-based for loop to check for error patterns
        for (const auto& [pattern, code] : error_map_)
        {
            if (error_msg.find(pattern) != std::string::npos)
            {
                response->set_error_code(code);
                break;
            }
        }

        return ::grpc::Status::OK;
    }
}
